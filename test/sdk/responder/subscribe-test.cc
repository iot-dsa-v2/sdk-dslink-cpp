#include "dsa/network.h"
#include "dsa/stream.h"

#include <gtest/gtest.h>
#include "../async_test.h"
#include "../test_config.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"
#include "responder/node_model.h"

using namespace dsa;

using ResponderTest = SetUpBase;

namespace responder_subscribe_test {
class MockNode : public NodeModelBase {
 public:
  std::unique_ptr<SubscribeOptions> first_subscribe_options;
  std::unique_ptr<SubscribeOptions> second_subscribe_options;
  bool need_subscribe() { return _need_subscribe; }

  explicit MockNode(const LinkStrandRef &strand) : NodeModelBase(strand){};

  void on_subscribe(const SubscribeOptions &options,
                    bool first_request) override {
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
    stream->send_subscribe_response(
        make_ref_<SubscribeResponseMessage>(Var("hello")));
    stream->on_subscribe_option_change(
        [=](MessageStream &stream, const SubscribeOptions &old_option) {
          if (stream.is_destroyed()) {
            unsubscribed = true;
            return;
          }
          last_subscribe_options.reset(
              new SubscribeOptions(stream.subscribe_options()));
        });
  }
  void add(ref_<OutgoingListStream> &&stream) override {}
  void add(ref_<OutgoingInvokeStream> &&stream) override {}
  void add(ref_<OutgoingSetStream> &&stream) override {}
  // TODO: keep in mind
  void custom_destroy() {
    // last_subscribe_stream->destroy();
    last_subscribe_stream.reset();
  }
};
}

TEST_F(ResponderTest, SubscribeModel) {
  typedef responder_subscribe_test::MockNode MockNode;
  auto app = std::make_shared<App>();

  TestConfig server_strand(app, false, protocol());

  MockNode *root_node = new MockNode(server_strand.strand);

  server_strand.strand->set_responder_model(ModelRef(root_node));

  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  auto web_server = server_strand.create_webserver();
  web_server->start();

  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();

  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  SubscribeOptions initial_options;
  initial_options.queue_duration = 0x1234;
  initial_options.queue_size = 0x5678;

  SubscribeOptions update_options;
  update_options.queue_duration = 0x9876;
  update_options.queue_size = 0x5432;

  int msg_count = 0;
  ref_<const SubscribeResponseMessage> last_response;
  auto subscribe_stream = tcp_client->get_session().subscribe(
      "",
      [&](IncomingSubscribeStream &stream,
          ref_<const SubscribeResponseMessage> &&msg) {
        last_response = std::move(msg);  // store response
        ++msg_count;
      },
      initial_options);

  // wait for root_node to receive the request
  ASYNC_EXPECT_TRUE(1000, *server_strand.strand, [&]() -> bool {
    return root_node->first_subscribe_options != nullptr;
  });

  // received request option should be same as the original one
  EXPECT_TRUE(initial_options == *root_node->first_subscribe_options);

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() -> bool { return last_response != nullptr; });

  EXPECT_TRUE(last_response->get_value().has_value() &&
              last_response->get_value().value.to_string() == "hello");

  // send an new request to update the option of the same stream
  subscribe_stream->subscribe(update_options);

  ASYNC_EXPECT_TRUE(1000, *server_strand.strand, [&]() -> bool {
    return root_node->second_subscribe_options != nullptr;
  });
  // request option should be same as the second one
  EXPECT_TRUE(update_options == *root_node->second_subscribe_options);

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand, [&]() -> bool {
    return last_response->get_value().value.get_string() == "world";
  });

  EXPECT_EQ(msg_count, 2);

  // close the subscribe stream
  subscribe_stream->close();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand, [&]() -> bool {
    return subscribe_stream->is_destroyed() &&
           subscribe_stream->ref_count() == 1;
  });

  ASYNC_EXPECT_TRUE(1000, *server_strand.strand,
                    [&]() -> bool { return !root_node->need_subscribe(); });

  tcp_server->destroy_in_strand(tcp_server);
  web_server->destroy();
  destroy_client_in_strand(tcp_client);

  server_strand.destroy();
  client_strand.destroy();
  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();
}

TEST_F(ResponderTest, SubscribeAcceptor) {
  typedef responder_subscribe_test::MockStreamAcceptor MockStreamAcceptor;
  auto app = std::make_shared<App>();

  MockStreamAcceptor *mock_stream_acceptor = new MockStreamAcceptor();

  TestConfig server_strand(app, false, protocol());
  server_strand.strand->set_stream_acceptor(
      ref_<MockStreamAcceptor>(mock_stream_acceptor));

  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  auto web_server = server_strand.create_webserver();
  web_server->start();

  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();

  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  SubscribeOptions initial_options;
  initial_options.queue_duration = 0x1234;
  initial_options.queue_size = 0x5678;

  SubscribeOptions update_options;
  update_options.queue_duration = 0x9876;
  update_options.queue_size = 0x5432;

  int msg_count = 0;
  ref_<const SubscribeResponseMessage> last_response;
  auto subscribe_stream = tcp_client->get_session().subscribe(
      "path",
      [&](IncomingSubscribeStream &stream,
          ref_<const SubscribeResponseMessage> &&msg) {
        last_response = std::move(msg);  // store response
        ++msg_count;
      },
      initial_options);

  // wait for acceptor to receive the request
  ASYNC_EXPECT_TRUE(1000, *server_strand.strand, [&]() -> bool {
    return mock_stream_acceptor->last_subscribe_stream != nullptr;
  });
  // received request option should be same as the original one
  EXPECT_TRUE(initial_options ==
              mock_stream_acceptor->last_subscribe_stream->subscribe_options());

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() -> bool { return last_response != nullptr; });

  EXPECT_TRUE(last_response->get_value().has_value() &&
              last_response->get_value().value.to_string() == "hello");

  EXPECT_EQ(msg_count, 1);

  // send an new request to udpate the option of the same stream
  subscribe_stream->subscribe(update_options);

  ASYNC_EXPECT_TRUE(1000, *server_strand.strand, [&]() -> bool {
    return mock_stream_acceptor->last_subscribe_options != nullptr;
  });
  // request option should be same as the second one
  EXPECT_TRUE(update_options == *mock_stream_acceptor->last_subscribe_options);
  EXPECT_TRUE(update_options ==
              mock_stream_acceptor->last_subscribe_stream->subscribe_options());

  // close the subscribe stream
  subscribe_stream->close();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand, [&]() -> bool {
    return subscribe_stream->is_destroyed() &&
           subscribe_stream->ref_count() == 1;
  });

  ASYNC_EXPECT_TRUE(1000, *server_strand.strand, [&]() -> bool {
    return mock_stream_acceptor->unsubscribed;
  });
  // subscribe_stream->destroy();
  tcp_server->destroy_in_strand(tcp_server);
  web_server->destroy();
  destroy_client_in_strand(tcp_client);

  server_strand.destroy();
  client_strand.destroy();
  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();
}
