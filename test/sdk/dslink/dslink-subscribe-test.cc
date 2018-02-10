
#include "dsa/network.h"
#include "dsa/stream.h"

#include <gtest/gtest.h>
#include "../async_test.h"
#include "../test_config.h"
#include "network/tcp/tcp_server.h"

using namespace dsa;
using namespace std;
using namespace boost::asio;

using DslinkTest = SetUpBase;

namespace link_subscribe_test {
class MockNode : public NodeModelBase {
 public:
  std::unique_ptr<SubscribeOptions> first_subscribe_options;
  std::unique_ptr<SubscribeOptions> second_subscribe_options;
  bool need_subscribe() { return _need_subscribe; }

  explicit MockNode(LinkStrandRef strand) : NodeModelBase(std::move(strand)){};

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

TEST_F(DslinkTest, SubscribeTest) {
  typedef link_subscribe_test::MockNode MockNode;

  shared_ptr<App> app = make_shared<App>();
  TestConfig server_strand(app, false, protocol());
  MockNode *root_node = new MockNode(server_strand.strand);
  server_strand.strand->set_responder_model(ModelRef(root_node));
  auto tcp_server = server_strand.create_server();
  tcp_server->start();
  auto web_server = server_strand.create_webserver();
  web_server->start();

  // Create link
  auto link = server_strand.create_dslink();

  std::vector<std::string> messages;
  // connection
  bool is_connected = false;
  link->connect([&](const shared_ptr_<Connection> connection,
                    ref_<DsLinkRequester> link_req) {
    // add a callback when connected to broker
    link_req->subscribe("", [&](IncomingSubscribeCache &cache,
                               ref_<const SubscribeResponseMessage> message) {
      messages.push_back(message->get_value().value.get_string());
    });

    is_connected = true;
  });

  ASYNC_EXPECT_TRUE(500, *link->strand, [&]() { return is_connected; });

  ASYNC_EXPECT_TRUE(500, *link->strand, [&]() { return messages.size() == 1; });

  EXPECT_TRUE(messages.size() == 1);
  EXPECT_EQ(messages.at(0), "hello");

  // Cleaning test
  tcp_server->destroy_in_strand(tcp_server);
  web_server->destroy();
  destroy_dslink_in_strand(link);

  server_strand.destroy();
  app->close();
  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();
};

TEST_F(DslinkTest, SubscribeMultiTest) {
  typedef link_subscribe_test::MockNode MockNode;

  auto app = make_shared<App>();
  TestConfig server_strand(app, false, protocol());
  MockNode *root_node = new MockNode(server_strand.strand);
  server_strand.strand->set_responder_model(ModelRef(root_node));
  auto tcp_server = server_strand.create_server();
  tcp_server->start();
  auto web_server = server_strand.create_webserver();
  web_server->start();

  // Create link
  auto link = server_strand.create_dslink();

  // connection
  bool is_connected = false;
  bool flag1 = false;
  bool flag2 = false;
  std::vector<std::string> messages_initial;
  std::vector<std::string> messages_updated;
  link->connect([&](const shared_ptr_<Connection> connection,
                    ref_<DsLinkRequester> link_req) {
    is_connected = true;

    // Initial Subcribe

    SubscribeOptions initial_options;
    initial_options.queue_duration = 0x1234;
    initial_options.queue_size = 0x5678;

    link_req->subscribe(
        "",
        [&, link_req = std::move(link_req)](IncomingSubscribeCache &cache,
            ref_<const SubscribeResponseMessage> message) {
          messages_initial.push_back(message->get_value().value.get_string());
          if (messages_initial.size() == 1) {
            // Subscribe update
            SubscribeOptions update_options;
            update_options.queue_duration = 0x9876;
            update_options.queue_size = 0x5432;

            link_req->subscribe(
                "",
                [&](IncomingSubscribeCache &cache,
                    ref_<const SubscribeResponseMessage> message) {
                  messages_updated.push_back(
                      message->get_value().value.get_string());
                  if (messages_updated.size() == 2) flag2 = true;
                },
                update_options);
          } else if (messages_initial.size() == 2) {
            flag1 = true;
          }
        },
        initial_options);
  });
  ASYNC_EXPECT_TRUE(3000, *link->strand,
                    [&]() { return (is_connected && flag1 && flag2); });

  EXPECT_STREQ(messages_initial.at(0).c_str(),
               "hello");  // received for the first subscribe request
  EXPECT_STREQ(messages_initial.at(1).c_str(),
               "world");  // received after value changed in the second request
  EXPECT_STREQ(messages_updated.at(0).c_str(),
               "hello");  // received just after the second subscribe request
  EXPECT_STREQ(messages_updated.at(1).c_str(),
               "world");  // received after value changed in the second request

  // Cleaning test
  tcp_server->destroy_in_strand(tcp_server);
  web_server->destroy();
  destroy_dslink_in_strand(link);

  server_strand.destroy();

  app->close();
  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();
};
