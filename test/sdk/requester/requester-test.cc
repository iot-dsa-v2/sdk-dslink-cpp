#include "dsa/network.h"
#include "dsa/stream.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"

using namespace dsa;

class MockStreamAcceptor : public OutgoingStreamAcceptor {
 public:
  ref_<OutgoingSubscribeStream> last_subscribe_stream;
  std::unique_ptr<SubscribeOptions> last_subscribe_options;
  void add(ref_<OutgoingSubscribeStream> &&stream) {
    BOOST_ASSERT_MSG(last_subscribe_stream == nullptr,
                     "receive second subscription stream, not expected");
    last_subscribe_stream = stream;
    stream->send_response(
        make_ref_<SubscribeResponseMessage>(Variant("hello")));
    stream->on_option_change([=](OutgoingSubscribeStream &stream,
                                 const SubscribeOptions &old_option) {
      last_subscribe_options.reset(new SubscribeOptions(stream.options()));
    });
  }
  void add(ref_<OutgoingListStream> &&stream) {}
  void add(ref_<OutgoingInvokeStream> &&stream) {}
  void add(ref_<OutgoingSetStream> &&stream) {}
};

TEST(RequesterTest, Subscribe) {
  App app;

  MockStreamAcceptor *mock_stream_acceptor = new MockStreamAcceptor();

  TestConfig server_config(app);
  server_config.get_link_config()->set_stream_acceptor(
      std::unique_ptr<MockStreamAcceptor>(mock_stream_acceptor));

  WrapperConfig client_config = server_config.get_client_config(app);

  //  auto tcp_server(new TcpServer(server_config));
  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  auto tcp_client = make_shared_<Client>(client_config);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  SubscribeOptions initial_options;
  initial_options.queue_time = 0x1234;
  initial_options.queue_size = 0x5678;

  SubscribeOptions update_options;
  update_options.queue_time = 0x9876;
  update_options.queue_size = 0x5432;

  ref_<const SubscribeResponseMessage> last_response;
  auto subscribe_stream = tcp_client->get_session().requester.subscribe(
      "path",
      [&](IncomingSubscribeStream &stream,
          ref_<const SubscribeResponseMessage> &&msg) {
        last_response = std::move(msg);  // store response
      },
      initial_options);

  // wait for acceptor to receive the request
  WAIT_EXPECT_TRUE(500, [&]() -> bool {
    return mock_stream_acceptor->last_subscribe_stream != nullptr;
  });
  // received request option should be same as the original one
  EXPECT_TRUE(initial_options ==
              mock_stream_acceptor->last_subscribe_stream->options());

  WAIT_EXPECT_TRUE(500, [&]() -> bool { return last_response != nullptr; });

  EXPECT_TRUE(last_response->get_value().has_value() &&
              last_response->get_value().value.is_string() &&
              last_response->get_value().value.get_string() == "hello");

  // send an new request to udpate the option of the same stream
  subscribe_stream->subscribe(update_options);

  WAIT_EXPECT_TRUE(500, [&]() -> bool {
    return mock_stream_acceptor->last_subscribe_options != nullptr;
  });
  // request option should be same as the second one
  EXPECT_TRUE(update_options == *mock_stream_acceptor->last_subscribe_options);
  EXPECT_TRUE(update_options ==
              mock_stream_acceptor->last_subscribe_stream->options());

  Server::close_in_strand(tcp_server);
  Client::close_in_strand(tcp_client);

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
