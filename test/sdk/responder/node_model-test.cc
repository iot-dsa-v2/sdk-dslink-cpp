#include "dsa/network.h"
#include "dsa/stream.h"

#include "../async_test.h"
#include "../test_config.h"
#include <gtest/gtest.h>

#include "core/client.h"
#include "network/tcp/tcp_server.h"

using namespace dsa;

class MockNode : public NodeModelBase {
 public:
  std::unique_ptr<SubscribeOptions> first_subscribe_options;
  std::unique_ptr<SubscribeOptions> second_subscribe_options;

  explicit MockNode(LinkStrandRef strand) : NodeModelBase(std::move(strand)){};

  void on_subscribe(const SubscribeOptions &options,
                    bool first_request) override {
    first_subscribe_options.reset(new SubscribeOptions(options));
    if (first_request) {
      set_value(Var("hello"));
    } else {
      set_value(Var("world"));
    }
  }
};

class MockStreamAcceptor : public OutgoingStreamAcceptor {
 public:
  ref_<OutgoingSubscribeStream> last_subscribe_stream;
  std::unique_ptr<SubscribeOptions> last_subscribe_options;
  void add(ref_<OutgoingSubscribeStream> &&stream) {
    BOOST_ASSERT_MSG(last_subscribe_stream == nullptr,
                     "receive second subscription stream, not expected");
    last_subscribe_stream = stream;
    stream->send_subscribe_response(make_ref_<SubscribeResponseMessage>(Var("hello")));
    stream->on_subscribe_option_change([=](MessageStream &stream,
                                           const SubscribeOptions &old_option) {
      last_subscribe_options.reset(new SubscribeOptions(stream.subscribe_options()));
    });
  }
  void add(ref_<OutgoingListStream> &&stream) {}
  void add(ref_<OutgoingInvokeStream> &&stream) {}
  void add(ref_<OutgoingSetStream> &&stream) {}
};

class MockNodeListChild_0 : public NodeModelBase {
 public:
  explicit MockNodeListChild_0(LinkStrandRef strand)
      : NodeModelBase(std::move(strand)){};
};

class MockNodeListRoot_0 : public NodeModelBase {
 public:
  std::unique_ptr<SubscribeOptions> first_subscribe_options;
  std::unique_ptr<SubscribeOptions> second_subscribe_options;

  explicit MockNodeListRoot_0(LinkStrandRef strand)
      : NodeModelBase(std::move(strand)){};

  void initialize() override {
    add_child("child_a", ref_<NodeModelBase>(new MockNodeListChild_0(_strand)));
  }
};

TEST(ResponderTest, model__add_child) {
  auto app = std::make_shared<App>();

  MockStreamAcceptor *mock_stream_acceptor = new MockStreamAcceptor();

  TestConfig server_strand(app);
  server_strand.strand->set_stream_acceptor(
      ref_<MockStreamAcceptor>(mock_stream_acceptor));

  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();

  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  SubscribeOptions initial_options;
  initial_options.queue_duration = 0x1234;
  initial_options.queue_size = 0x5678;

  // test invalid path scenario
  auto subscribe_stream = tcp_client->get_session().requester.subscribe(
      "child_a", [&](IncomingSubscribeStream &stream,
                     ref_<const SubscribeResponseMessage> &&msg) { ; },
      initial_options);

  wait_for_bool(25, [&]() -> bool { return false; });

  tcp_server->destroy_in_strand(tcp_server);
  destroy_client_in_strand(tcp_client);

  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  server_strand.destroy();
  client_strand.destroy();
  app->wait();
}

TEST(ResponderTest, model__get_child) {
  auto app = std::make_shared<App>();

  TestConfig server_strand(app);

  MockNode *root_node = new MockNode(server_strand.strand);

  server_strand.strand->set_responder_model(ref_<MockNode>(root_node));

  //  auto tcp_server(new TcpServer(server_strand));
  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();

  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  ModelRef child_node = root_node->get_child("child_a");

  EXPECT_EQ(nullptr, child_node);

  root_node->add_child("child_a",
                       ModelRef(new MockNodeListChild_0(server_strand.strand)));
  child_node = root_node->get_child("child_a");

  EXPECT_NE(nullptr, child_node);

  tcp_server->destroy_in_strand(tcp_server);
  destroy_client_in_strand(tcp_client);

  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  child_node->destroy();
  server_strand.destroy();
  client_strand.destroy();
  app->wait();
}

TEST(ResponderTest, model__set_value) {
  auto app = std::make_shared<App>();

  TestConfig server_strand(app);

  MockNode *root_node = new MockNode(server_strand.strand);

  server_strand.strand->set_responder_model(ref_<MockNode>(root_node));

  //  auto tcp_server(new TcpServer(server_strand));
  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();

  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  SubscribeOptions initial_options;
  initial_options.queue_duration = 0x1234;
  initial_options.queue_size = 0x5678;

  root_node->set_value(MessageValue(Var(0)));

  auto subscribe_stream = tcp_client->get_session().requester.subscribe(
      "", [&](IncomingSubscribeStream &stream,
              ref_<const SubscribeResponseMessage> &&msg) { ; },
      initial_options);

  //
  wait_for_bool(25, [&]() -> bool { return false; });

  SubscribeResponseMessageCRef cached_message =
      make_ref_<SubscribeResponseMessage>(Var(0));
  root_node->set_subscribe_response(copy_ref_(cached_message));

  tcp_server->destroy_in_strand(tcp_server);
  destroy_client_in_strand(tcp_client);

  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  server_strand.destroy();
  client_strand.destroy();
  app->wait();
}
