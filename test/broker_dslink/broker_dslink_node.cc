#include "broker_dslink_util.h"

using BrokerDsLinkTest = SetUpBase;

class MockChildNode : public NodeModel {
 public:
  explicit MockChildNode(LinkStrandRef strand) : NodeModel(std::move(strand)) {
    set_value(Var("child value"));
  };
};

class MockNode : public NodeModel {
 public:
  explicit MockNode(LinkStrandRef strand) : NodeModel(std::move(strand)) {
    add_list_child("Child_Node", make_ref_<MockChildNode>(_strand));
  };

  void on_subscribe(const SubscribeOptions &options,
                    bool first_request) override {
    if (first_request) {
      set_value(Var("child value"));
    } else {
      remove_list_child("Child_Node");
    }
  }
};

TEST_F(BrokerDsLinkTest, RemoveNodeList) {
  // First Create Broker
  auto app = make_shared_<App>();
  auto broker = broker_dslink_test::create_broker(app);
  broker->run();

  int32_t port;

  switch (protocol()) {
    case dsa::ProtocolType::PROT_DSS:
      port = broker->get_active_secure_port();
      break;
    default:
      port = broker->get_active_server_port();
  }

  EXPECT_TRUE(port != 0);

  auto link_1 = broker_dslink_test::create_mock_dslink(app, port, "Test1", true,
                                                       protocol());
  auto link_2 =
      broker_dslink_test::create_dslink(app, port, "Test2", false, protocol());
  link_1->add_to_main_node("Main_Child",
                           make_ref_<MockNode>(link_1->strand->get_ref()));

  bool test_end = false, first_list = false;

  link_2->connect([&](const shared_ptr_<Connection> connection,
                      ref_<DsLinkRequester> link_req) {
    // downstream should has test1 and test2 nodes
    link_req->list(
        "Downstream",
        [
              &,
              link_req = static_cast<ref_<DsLinkRequester>>(link_req->get_ref())
        ](IncomingListCache & cache, const std::vector<string_> &str) {
          auto map = cache.get_map();
          EXPECT_TRUE(map["Test1"].is_map());
          EXPECT_TRUE(map["Test2"].is_map());
          cache.close();

          link_req->list(
              "Downstream/Test1/Main",
              [
                    &,
                    link_req =
                        static_cast<ref_<DsLinkRequester>>(link_req->get_ref())
              ](IncomingListCache & cache, const std::vector<string_> &str) {
                auto map = cache.get_map();

                EXPECT_TRUE(map.find("Child_a") != map.end());
                EXPECT_TRUE(map.find("Main_Child") != map.end());
                if (map.find("Child_b") != map.end()) {
                  link_1->remove_from_main_node("Child_b");
                  first_list = true;
                } else if (map.find("Child_b") == map.end()) {
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

  int32_t port;

  switch (protocol()) {
    case dsa::ProtocolType::PROT_DSS:
      port = broker->get_active_secure_port();
      break;
    default:
      port = broker->get_active_server_port();
  }

  EXPECT_TRUE(port != 0);

  auto link_1 = broker_dslink_test::create_mock_dslink(app, port, "Test1", true,
                                                       protocol());
  auto link_2 =
      broker_dslink_test::create_dslink(app, port, "Test2", false, protocol());
  link_1->add_to_main_node("Main_Child",
                           make_ref_<MockNode>(link_1->strand->get_ref()));

  bool test_end = false;

  link_2->connect([&](const shared_ptr_<Connection> connection,
                      ref_<DsLinkRequester> link_req) {
    // downstream should has test1 and test2 nodes
    link_req->list(
        "Downstream",
        [
              &,
              link_req = static_cast<ref_<DsLinkRequester>>(link_req->get_ref())
        ](IncomingListCache & cache, const std::vector<string_> &str) {
          auto map = cache.get_map();
          EXPECT_TRUE(map["Test1"].is_map());
          EXPECT_TRUE(map["Test2"].is_map());
          cache.close();

          // after client1 disconnected, list update should show it's
          // disconnected
          link_req->list(
              "Downstream/Test1/Main",
              [
                    &,
                    link_req =
                        static_cast<ref_<DsLinkRequester>>(link_req->get_ref())
              ](IncomingListCache & cache, const std::vector<string_> &str) {
                auto map = cache.get_map();
                EXPECT_TRUE(map["Main_Child"].is_map());
                cache.close();
                link_1->remove_from_main_node("Main_Child");

                link_req->subscribe(
                    "Downstream/Test1/Main/Main_Child",
                    [&](IncomingSubscribeCache &cache,
                        ref_<const SubscribeResponseMessage> &msg) {
                      EXPECT_EQ(msg->get_status(),
                                MessageStatus::NOT_SUPPORTED);
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
