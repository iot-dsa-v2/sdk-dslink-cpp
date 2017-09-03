#include "dsa/message.h"
#include "dsa/network.h"

#include "../async_test.h"
#include "gtest/gtest.h"

#include "network/tcp/tcp_client.h"
#include "network/tcp/tcp_server.h"

#include "module/default_modules.h"

using namespace dsa;

class MockStreamAcceptor : public OutgoingStreamAcceptor {
 public:
  ref_<OutgoingSubscribeStream> last_subscribe_stream;
  void add(ref_<OutgoingSubscribeStream> &stream) {
    last_subscribe_stream = stream;
    stream->send_value(Variant("hello"));
  }
  void add(ref_<OutgoingListStream> &stream) {}
  void add(ref_<OutgoingInvokeStream> &stream) {}
  void add(ref_<OutgoingSetStream> &stream) {}
};

TEST(RequesterTest, Subscribe) {
  App app;

  // capture and log request
  auto mock_stream_acceptor = make_shared_<MockStreamAcceptor>();

  auto modules = make_ref_<DefaultModules>(app);
  modules->set_stream_acceptor(mock_stream_acceptor);

  WrapperConfig server_config;
  server_config.tcp_host = "127.0.0.1";
  server_config.tcp_port = 8092;
  server_config.strand = std::move(modules);
  server_config.strand->logger().level = Logger::WARNING;

  app.async_start(10);

  //  auto tcp_server(new TcpServer(server_config));
  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  WrapperConfig client_config;
  client_config.tcp_host = "127.0.0.1";
  client_config.tcp_port = 8092;
  client_config.strand = make_ref_<DefaultModules>(app);
  client_config.strand->logger().level = Logger::WARNING;

  auto tcp_client = make_shared_<TcpClient>(client_config);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(500000, (*client_config.strand)(),
                    [&]() { return tcp_client->get_session().is_connected(); });

  SubscribeOptions options;
  options.queue_time = 0x1234;
  options.queue_size = 0x5678;

  ref_<SubscribeResponseMessage> last_response;
  tcp_client->get_session().requester.subscribe(
      "/path",
      [&](ref_<SubscribeResponseMessage> &&msg,
          IncomingSubscribeStream &stream) {
        last_response = std::move(msg);
      },
      options);

  // wait for acceptor to receive the request
  WAIT_EXPECT_TRUE(500000, [&]() -> bool {
    return mock_stream_acceptor->last_subscribe_stream != nullptr;
  });

  // received request option should be same as the original one
  EXPECT_TRUE(options == mock_stream_acceptor->last_subscribe_stream->options);

  WAIT_EXPECT_TRUE(500000, [&]() -> bool { return last_response != nullptr; });

  EXPECT_TRUE(last_response->get_value().has_value() &&
              last_response->get_value().value.is_string() &&
              last_response->get_value().value.get_string() == "hello");

  Server::close_in_strand(tcp_server);
  Client::close_in_strand(tcp_client);

  app.close();

  WAIT_EXPECT_TRUE(500000, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
