#include "broker_dslink_util.h"

using BrokerDsLinkTest = SetUpBase;

class MockChildNode : public NodeModel {
 public:
  explicit MockChildNode(const LinkStrandRef &strand) : NodeModel(strand) {
    set_value(Var("child value"));
  };
};

class MockNode : public NodeModel {
 public:
  explicit MockNode(const LinkStrandRef &strand) : NodeModel(strand) {
    add_list_child("child-node", make_ref_<MockChildNode>(_strand));
  };

  void on_subscribe(const SubscribeOptions &options,
                    bool first_request) override {
    if (first_request) {
      set_value(Var("child value"));
    } else {
      remove_list_child("child-node");
    }
  }
};

TEST_F(BrokerDsLinkTest, RemoveNodeList) {
  // First Create Broker
  auto app = make_shared_<App>();
  auto broker = broker_dslink_test::create_broker(app);
  broker->run();
  ASYNC_EXPECT_TRUE(1000, *broker->strand,
                    [&]() { return broker->get_active_server_port() != 0; });
  int32_t port;

  switch (protocol()) {
    case dsa::ProtocolType::PROT_DSS:
      port = broker->get_active_secure_port();
      break;
    default:
      port = broker->get_active_server_port();
  }

  EXPECT_TRUE(port != 0);

  auto link_1 = broker_dslink_test::create_mock_dslink(app, port, "test1", true,
                                                       protocol());
  auto link_2 =
      broker_dslink_test::create_dslink(app, port, "test2", false, protocol());
  link_1->add_to_main_node("main-child",
                           make_ref_<MockNode>(link_1->strand));

  bool test_end = false, first_list = false;

  link_2->connect([&](const shared_ptr_<Connection> connection,
                      ref_<DsLinkRequester> link_req) {
    // downstream should has test1 and test2 nodes
    link_req->list(
        "downstream",
        [
              &,
              link_req = static_cast<ref_<DsLinkRequester>>(link_req->get_ref())
        ](IncomingListCache & cache, const std::vector<string_> &str) {
          auto map = cache.get_map();
          EXPECT_TRUE(map["test1"].is_map());
          EXPECT_TRUE(map["test2"].is_map());
          cache.close();

          link_req->list(
              "downstream/test1/main",
              [
                    &,
                    link_req =
                        static_cast<ref_<DsLinkRequester>>(link_req->get_ref())
              ](IncomingListCache & cache, const std::vector<string_> &str) {
                auto map = cache.get_map();

                EXPECT_TRUE(map.find("child-a") != map.end());
                EXPECT_TRUE(map.find("main-child") != map.end());
                if (map.find("child-b") != map.end()) {
                  link_1->remove_from_main_node("child-b");
                  first_list = true;
                } else if (map.find("child-b") == map.end()) {
                  test_end = true;
                  cache.close();
                }
              });
        });
  });

  WAIT_EXPECT_TRUE(5000, [&]() -> bool { return first_list && test_end; });

  link_1->strand->post([link_1]() { link_1->destroy(); });
  link_2->strand->post([link_2]() { link_2->destroy(); });
  broker->strand->post([broker]() { broker->destroy(); });
  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();
}
TEST_F(BrokerDsLinkTest, RemoveNodeSubcribe) {
  // First Create Broker
  auto app = make_shared_<App>();
  auto broker = broker_dslink_test::create_broker(app);
  broker->run();
  ASYNC_EXPECT_TRUE(1000, *broker->strand,
                    [&]() { return broker->get_active_server_port() != 0; });
  int32_t port;

  switch (protocol()) {
    case dsa::ProtocolType::PROT_DSS:
      port = broker->get_active_secure_port();
      break;
    default:
      port = broker->get_active_server_port();
  }

  EXPECT_TRUE(port != 0);

  auto link_1 = broker_dslink_test::create_mock_dslink(app, port, "test1", true,
                                                       protocol());
  auto link_2 =
      broker_dslink_test::create_dslink(app, port, "test2", false, protocol());
  link_1->add_to_main_node("main-child",
                           make_ref_<MockNode>(link_1->strand));

  bool test_end = false;

  link_2->connect([&](const shared_ptr_<Connection> connection,
                      ref_<DsLinkRequester> link_req) {
    // downstream should has test1 and test2 nodes
    link_req->list(
        "downstream",
        [
              &,
              link_req = static_cast<ref_<DsLinkRequester>>(link_req->get_ref())
        ](IncomingListCache & cache, const std::vector<string_> &str) {
          auto map = cache.get_map();
          EXPECT_TRUE(map["test1"].is_map());
          EXPECT_TRUE(map["test2"].is_map());
          cache.close();

          // after client1 disconnected, list update should show it's
          // disconnected
          link_req->list(
              "downstream/test1/main",
              [
                    &,
                    link_req =
                        static_cast<ref_<DsLinkRequester>>(link_req->get_ref())
              ](IncomingListCache & cache, const std::vector<string_> &str) {
                auto map = cache.get_map();
                EXPECT_TRUE(map["main-child"].is_map());
                cache.close();
                link_1->remove_from_main_node("main-child");

                link_req->subscribe(
                    "downstream/test1/main/main-child",
                    [&](IncomingSubscribeCache &cache,
                        ref_<const SubscribeResponseMessage> &msg) {
                      EXPECT_EQ(msg->get_status(),
                                Status::NOT_SUPPORTED);
                      test_end = true;
                    });

              });
        });
  });

  WAIT_EXPECT_TRUE(5000, [&]() -> bool { return test_end; });

  link_1->strand->post([link_1]() { link_1->destroy(); });
  link_2->strand->post([link_2]() { link_2->destroy(); });
  broker->strand->post([broker]() { broker->destroy(); });
  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();
}
