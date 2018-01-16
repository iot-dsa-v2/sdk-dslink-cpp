
#include "dsa/network.h"
#include "dsa/stream.h"

#include "../async_test.h"
#include "../test_config.h"
#include <gtest/gtest.h>
#include "network/tcp/tcp_server.h"

using namespace dsa;
using namespace std;
using namespace boost::asio;

namespace link_subscribe_test {
class MockNode : public NodeModelBase {
 public:
  std::unique_ptr<SubscribeOptions> first_subscribe_options;
  std::unique_ptr<SubscribeOptions> second_subscribe_options;
  bool need_subscribe() { return _need_subscribe; }

  explicit MockNode(LinkStrandRef strand) : NodeModelBase(std::move(strand)) {};

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

}

TEST(DSLinkTest, Subscribe_Test) {
  typedef link_subscribe_test::MockNode MockNode;

  shared_ptr<App> app = make_shared<App>();
  TestConfig server_strand(app);
  MockNode *root_node = new MockNode(server_strand.strand);
  server_strand.strand->set_responder_model(ModelRef(root_node));
  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  // Create link
  auto link = server_strand.create_dslink();

  // connection
  bool is_connected = false;
  link->connect([&](const shared_ptr_<Connection> connection) { is_connected = true; });

  ASYNC_EXPECT_TRUE(500, *link->strand, [&]() {return is_connected;});

  // add a callback when connected to broker
  std::vector<std::string> messages;

  link->subscribe("",
                  [&](IncomingSubscribeCache &cache, ref_<const SubscribeResponseMessage> message) {
                      messages.push_back(message->get_value().value.get_string()); });

  ASYNC_EXPECT_TRUE(500, *link.get()->strand,
                    [&]() { return messages.size() == 1; });

  EXPECT_TRUE(messages.size() == 1);
  EXPECT_EQ(messages.at(0), "hello");

  // Cleaning test
  tcp_server->destroy_in_strand(tcp_server);
  destroy_dslink_in_strand(link);

  app->close();
  WAIT_EXPECT_TRUE(500, [&]() -> bool  { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  server_strand.destroy();

  app->wait();

};

TEST(LinkTest, Subscribe_Multi_Test) {
  typedef link_subscribe_test::MockNode MockNode;

  auto app = make_shared<App>();
  TestConfig server_strand(app);
  MockNode *root_node = new MockNode(server_strand.strand);
  server_strand.strand->set_responder_model(ModelRef(root_node));
  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  // Create link
  auto link = server_strand.create_dslink();

  // connection
  bool is_connected = false;
  link->connect([&](const shared_ptr_<Connection> connection) { is_connected = true; });
  ASYNC_EXPECT_TRUE(500, *link->strand, [&]() {return is_connected;});

  // Initial Subcribe
  std::vector<std::string> messages_initial;

  SubscribeOptions initial_options;
  initial_options.queue_duration = 0x1234;
  initial_options.queue_size = 0x5678;

  link->subscribe("",
                  [&](IncomingSubscribeCache &cache, ref_<const SubscribeResponseMessage> message) {
                      messages_initial.push_back(message->get_value().value.get_string()); },
                  initial_options);
  ASYNC_EXPECT_TRUE(500, *link.get()->strand,
                    [&]() { return messages_initial.size() == 1; });

  // Subscribe update
  std::vector<std::string> messages_updated;

  SubscribeOptions update_options;
  update_options.queue_duration = 0x9876;
  update_options.queue_size = 0x5432;

  link->subscribe("",
                  [&](IncomingSubscribeCache &cache, ref_<const SubscribeResponseMessage> message) {
                      messages_updated.push_back(message->get_value().value.get_string()); },
                  update_options);

  ASYNC_EXPECT_TRUE(500, *link.get()->strand,
                    [&]() { return messages_initial.size() == 2; });

  // TODO: How to Check return messages
  EXPECT_EQ(messages_updated.size(), 2);
  EXPECT_STREQ(messages_initial.at(0).c_str(), "hello"); // received for the first subscribe request
  EXPECT_STREQ(messages_initial.at(1).c_str(), "world"); // received after value changed in the second request
  EXPECT_STREQ(messages_updated.at(0).c_str(), "hello"); // received just after the second subscribe request
  EXPECT_STREQ(messages_updated.at(1).c_str(), "world"); // received after value changed in the second request

  // Cleaning test
  tcp_server->destroy_in_strand(tcp_server);
  destroy_dslink_in_strand(link);

  app->close();
  WAIT_EXPECT_TRUE(500, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  server_strand.destroy();

  app->wait();
};

