
#include "broker_dslink_util.h"

using BrokerDsLinkTest = SetUpBase;

TEST_F(BrokerDsLinkTest, RootSysSelfList) {
  std::string close_token = "12345678901234567890123456789012";
  string_to_file(close_token, ".close_token");

  typedef std::vector<std::vector<string_>> ListResponses;

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

  auto link =
      broker_dslink_test::create_mock_dslink(app, port, "test1", protocol());

  bool is_connected = false;
  link->connect(
      [&](const shared_ptr_<Connection> connection) { is_connected = true; });
  ASYNC_EXPECT_TRUE(1000, *link->strand, [&]() { return is_connected; });

  // list on root node
  ListResponses root_list_responses;
  bool listed = false;
  link->list("",
             [&](IncomingListCache &cache, const std::vector<string_> &str) {
               root_list_responses.push_back(str);
               VarMap map = cache.get_map();
               EXPECT_TRUE(root_list_responses.size() == 1);
               EXPECT_TRUE(root_list_responses[0].size() == 0);
               EXPECT_TRUE(map["downstream"].is_map());
               EXPECT_TRUE(map["home"].is_map());
               EXPECT_TRUE(map["pub"].is_map());
               EXPECT_TRUE(map["sys"].is_map());
               EXPECT_TRUE(map["upstream"].is_map());
               listed = true;
             });

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return listed; });

  // list on child node
  listed = false;
  link->list("downstream",
             [&](IncomingListCache &cache, const std::vector<string_> &str) {
               VarMap downstream_map = cache.get_map();
               EXPECT_TRUE(downstream_map["test1"].is_map());
               listed = true;
             });

  WAIT_EXPECT_TRUE(1000, [&]() { return listed; });

  // list on sys
  listed = false;
  link->list("sys",
             [&](IncomingListCache &cache, const std::vector<string_> &str) {
               VarMap sys_map = cache.get_map();
               EXPECT_TRUE(sys_map["stop"].is_map());
               listed = true;
             });

  WAIT_EXPECT_TRUE(1000, [&]() { return listed; });

  // list on self
  listed = false;
  link->list("downstream/test1",
             [&](IncomingListCache &cache, const std::vector<string_> &str) {
               VarMap self_map = cache.get_map();
               EXPECT_TRUE(self_map["$$dsid"].is_string());
               EXPECT_TRUE(self_map["main"].is_map());
               EXPECT_TRUE(self_map["pub"].is_map());
               EXPECT_TRUE(self_map["sys"].is_map());
               listed = true;
             });

  WAIT_EXPECT_TRUE(1000, [&]() { return listed; });

  link->strand->post([&]() { link->destroy(); });
  broker->strand->post([&]() { broker->destroy(); });

  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();
}

TEST_F(BrokerDsLinkTest, Disconnect) {
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

  auto link_1 =
      broker_dslink_test::create_dslink(app, port, "test1", false, protocol());
  auto link_2 =
      broker_dslink_test::create_dslink(app, port, "test2", false, protocol());

  // after client1 disconnected, list update should show it's disconnected
  auto step_3_disconnection_list = [&]() {
    link_2->list("downstream/test1", [&](IncomingListCache &cache,
                                         const std::vector<string_> &str) {
      EXPECT_EQ(cache.get_status(), MessageStatus::NOT_AVAILABLE);
      // end the test

      link_2->strand->post([link_2]() { link_2->destroy(); });
      broker->strand->post([broker]() { broker->destroy(); });
    });
  };

  // downstream should has test1 and test2 nodes
  auto step_2_downstream_list = [&]() {
    link_2->list("downstream", [&](IncomingListCache &cache,
                                   const std::vector<string_> &str) {
      auto map = cache.get_map();
      EXPECT_TRUE(map["test1"].is_map());
      EXPECT_TRUE(map["test2"].is_map());
      step_3_disconnection_list();
    });
  };

  // when list on downstream/test1 it should have a metadata for test1's dsid
  auto step_1_downstream_child_list = [&]() {
    link_1->list("downstream/test1", [&](IncomingListCache &cache,
                                         const std::vector<string_> &str) {
      auto map = cache.get_map();
      // std::cout<<"dsid : "<< map["$$dsid"].get_string()<<std::endl;
      //              EXPECT_EQ(map["$$dsid"].to_string(), link_1->dsid());

      link_1->strand->post([link_1]() { link_1->destroy(); });
      step_2_downstream_list();
    });

  };

  std::mutex mutex;
  bool one_of_them_connected = false;
  link_1->connect([&](const shared_ptr_<Connection> connection) {
    // std::cout<<"Hello1"<<std::endl;
    std::lock_guard<std::mutex> lock{mutex};
    if (one_of_them_connected) step_1_downstream_child_list();
    one_of_them_connected = true;
  });
  link_2->connect([&](const shared_ptr_<Connection> connection) {
    // std::cout<<"Hello2"<<std::endl;
    std::lock_guard<std::mutex> lock{mutex};
    if (one_of_them_connected) step_1_downstream_child_list();
    one_of_them_connected = true;
  });

  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();
}
