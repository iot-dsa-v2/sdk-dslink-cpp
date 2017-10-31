#include "dsa/network.h"
#include "dsa/stream.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"

using namespace dsa;

namespace responder_subscribe_test {
class MockNode : public NodeModelBase {
public:
  std::unique_ptr<SubscribeOptions> first_subscribe_options;
  std::unique_ptr<SubscribeOptions> second_subscribe_options;
  bool need_subscribe(){return _need_subscribe;}

  explicit MockNode(LinkStrandRef strand) : NodeModelBase(std::move(strand)){};

  void on_subscribe(const SubscribeOptions &options, bool first_request) override {
    if (first_request) {
      first_subscribe_options.reset(new SubscribeOptions(options));
      set_value(Var("hello"));
    } else {
      second_subscribe_options.reset(new SubscribeOptions(options));
      set_value(Var("world"));
    }
  }
};
class MockStreamAcceptor : public OutgoingStreamAcceptor {
public:
  ref_<OutgoingSubscribeStream> last_subscribe_stream;
  std::unique_ptr<SubscribeOptions> last_subscribe_options;
  bool unsubscribed = false;
  void add(ref_<OutgoingSubscribeStream> &&stream) {
    BOOST_ASSERT_MSG(last_subscribe_stream == nullptr,
                     "receive second subscription stream, not expected");
    last_subscribe_stream = stream;
    stream->send_response(make_ref_<SubscribeResponseMessage>(Var("hello")));
    stream->on_option_change([=](OutgoingSubscribeStream &stream,
                                 const SubscribeOptions &old_option) {
      if (stream.is_destroyed()) {
        unsubscribed = true;
        return;
      }
      last_subscribe_options.reset(new SubscribeOptions(stream.options()));
    });
  }
  void add(ref_<OutgoingListStream> &&stream) override {}
  void add(ref_<OutgoingInvokeStream> &&stream) override {}
  void add(ref_<OutgoingSetStream> &&stream) override {}
};
}

TEST(ResponderTest, Subscribe_Model) {
  typedef responder_subscribe_test::MockNode MockNode;
  App app;

  TestConfig server_config(app);

  MockNode *root_node = new MockNode(server_config.strand);

  server_config.strand->set_responder_model(ModelRef(root_node));

  WrapperConfig client_config = server_config.get_client_config(app);

  //  auto tcp_server(new TcpServer(server_config));
  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  auto tcp_client = make_ref_<Client>(client_config);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  SubscribeOptions initial_options;
  initial_options.queue_duration = 0x1234;
  initial_options.queue_size = 0x5678;

  SubscribeOptions update_options;
  update_options.queue_duration = 0x9876;
  update_options.queue_size = 0x5432;

  ref_<const SubscribeResponseMessage> last_response;
  auto subscribe_stream = tcp_client->get_session().requester.subscribe(
    "",
    [&](IncomingSubscribeStream &stream,
        ref_<const SubscribeResponseMessage> &&msg) {
      last_response = std::move(msg);  // store response
    },
    initial_options);

  // wait for root_node to receive the request
  ASYNC_EXPECT_TRUE(500, *server_config.strand, [&]() -> bool {
    return root_node->first_subscribe_options != nullptr;
  });

  // received request option should be same as the original one
  EXPECT_TRUE(initial_options == *root_node->first_subscribe_options);

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() -> bool { return last_response != nullptr; });

  EXPECT_TRUE(last_response->get_value().has_value() &&
              last_response->get_value().value.to_string() == "hello");

  // send an new request to update the option of the same stream
  subscribe_stream->subscribe(update_options);

  ASYNC_EXPECT_TRUE(500, *server_config.strand, [&]() -> bool {
    return root_node->second_subscribe_options != nullptr;
  });
  // request option should be same as the second one
  EXPECT_TRUE(update_options == *root_node->second_subscribe_options);

  ASYNC_EXPECT_TRUE(500, *client_config.strand, [&]() -> bool {
    return last_response->get_value().value.get_string() == "world";
  });

  // close the subscribe stream
  subscribe_stream->close();

  ASYNC_EXPECT_TRUE(500, *client_config.strand, [&]() -> bool {
    return subscribe_stream->is_destroyed() &&
           subscribe_stream->ref_count() == 1;
  });

  ASYNC_EXPECT_TRUE(500, *server_config.strand,
                    [&]() -> bool { return !root_node->need_subscribe(); });

  tcp_server->destroy_in_strand(tcp_server);
destroy_client_in_strand(tcp_client);

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}

TEST(ResponderTest, Subscribe_Acceptor) {
  typedef responder_subscribe_test::MockStreamAcceptor MockStreamAcceptor;
  App app;

  MockStreamAcceptor *mock_stream_acceptor = new MockStreamAcceptor();

  TestConfig server_config(app);
  server_config.strand->set_stream_acceptor(
    std::unique_ptr<MockStreamAcceptor>(mock_stream_acceptor));

  WrapperConfig client_config = server_config.get_client_config(app, true);

  //  auto tcp_server(new TcpServer(server_config));
  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  auto tcp_client = make_ref_<Client>(client_config);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  SubscribeOptions initial_options;
  initial_options.queue_duration = 0x1234;
  initial_options.queue_size = 0x5678;

  SubscribeOptions update_options;
  update_options.queue_duration = 0x9876;
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
  ASYNC_EXPECT_TRUE(500, *server_config.strand, [&]() -> bool {
    return mock_stream_acceptor->last_subscribe_stream != nullptr;
  });
  // received request option should be same as the original one
  EXPECT_TRUE(initial_options ==
              mock_stream_acceptor->last_subscribe_stream->options());

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() -> bool { return last_response != nullptr; });

  EXPECT_TRUE(last_response->get_value().has_value() &&
              last_response->get_value().value.to_string() == "hello");

  // send an new request to udpate the option of the same stream
  subscribe_stream->subscribe(update_options);

  ASYNC_EXPECT_TRUE(500, *server_config.strand, [&]() -> bool {
    return mock_stream_acceptor->last_subscribe_options != nullptr;
  });
  // request option should be same as the second one
  EXPECT_TRUE(update_options == *mock_stream_acceptor->last_subscribe_options);
  EXPECT_TRUE(update_options ==
              mock_stream_acceptor->last_subscribe_stream->options());

  // close the subscribe stream
  subscribe_stream->close();

  ASYNC_EXPECT_TRUE(500, *client_config.strand, [&]() -> bool {
    return subscribe_stream->is_destroyed() &&
           subscribe_stream->ref_count() == 1;
  });

  ASYNC_EXPECT_TRUE(500, *server_config.strand, [&]() -> bool {
    return mock_stream_acceptor->unsubscribed;
  });

  tcp_server->destroy_in_strand(tcp_server);
destroy_client_in_strand(tcp_client);

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
