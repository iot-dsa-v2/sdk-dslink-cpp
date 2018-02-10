
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
  bool listed_1 = false;
  bool listed_2 = false;
  bool listed_3 = false;
  bool listed_4 = false;

  link->connect([&](const shared_ptr_<Connection> connection,
                    ref_<DsLinkRequester> link_req) {
    is_connected = true;  // list on root node
    link_req->list(
        "", [&](IncomingListCache &cache, const std::vector<string_> &str) {
          VarMap map = cache.get_map();
          EXPECT_TRUE(map["downstream"].is_map());
          EXPECT_TRUE(map["home"].is_map());
          EXPECT_TRUE(map["pub"].is_map());
          EXPECT_TRUE(map["sys"].is_map());
          EXPECT_TRUE(map["upstream"].is_map());
          listed_1 = true;
          cache.close();
        });

    // list on child node
    link_req->list("downstream", [&](IncomingListCache &cache,
                                    const std::vector<string_> &str) {
      VarMap downstream_map = cache.get_map();
      EXPECT_TRUE(downstream_map["test1"].is_map());
      listed_2 = true;
      cache.close();
    });

    // list on sys
    link_req->list(
        "sys", [&](IncomingListCache &cache, const std::vector<string_> &str) {
          VarMap sys_map = cache.get_map();
          EXPECT_TRUE(sys_map["stop"].is_map());
          listed_3 = true;
          cache.close();
        });

    // list on self
    link_req->list("downstream/test1", [&](IncomingListCache &cache,
                                          const std::vector<string_> &str) {
      VarMap self_map = cache.get_map();
      EXPECT_TRUE(self_map["$$dsid"].is_string());
      EXPECT_TRUE(self_map["main"].is_map());
      EXPECT_TRUE(self_map["pub"].is_map());
      EXPECT_TRUE(self_map["sys"].is_map());
      listed_4 = true;
      cache.close();
    });
  });
  ASYNC_EXPECT_TRUE(2000, *link->strand, [&]() {
    return is_connected && listed_1 && listed_2 && listed_3 && listed_4;
  });
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

  bool link1_listed = false, link1_connected = false, test_end = false;
  link_1->connect(
      [&](const shared_ptr_<Connection> connection, ref_<DsLinkRequester> link_req) {
        link1_connected = true;
        // when list on downstream/test1 it should have a metadata for test1's
        // dsid
        link_req->list("downstream/test1", [&](IncomingListCache &cache,
                                              const std::vector<string_> &str) {
          auto map = cache.get_map();
          // std::cout<<"dsid : "<< map["$$dsid"].get_string()<<std::endl;
          //              EXPECT_EQ(map["$$dsid"].to_string(), link_1->dsid());

          link_1->strand->post([link_1]() { link_1->destroy(); });
          link1_listed = true;
        });
      });
  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return link1_connected; });

  link_2->connect([&](const shared_ptr_<Connection> connection,
                      ref_<DsLinkRequester> link_req) {
    // downstream should has test1 and test2 nodes
    link_req->list("downstream", [&, link_req = std::move(link_req)](IncomingListCache &cache,
                                    const std::vector<string_> &str) {
      auto map = cache.get_map();
      EXPECT_TRUE(map["test1"].is_map());
      EXPECT_TRUE(map["test2"].is_map());

      ASYNC_EXPECT_TRUE(1000, *link_1->strand,
                        [&]() -> bool { return link1_listed; });
      // after client1 disconnected, list update should show it's disconnected
      link_req->list("downstream/test1", [&](IncomingListCache &cache,
                                            const std::vector<string_> &str) {
        EXPECT_EQ(cache.get_status(), MessageStatus::NOT_AVAILABLE);
        // end the test

        link_2->strand->post([link_2]() { link_2->destroy(); });
        broker->strand->post([broker]() { broker->destroy(); });
        test_end = true;
      });
    });
  });

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return test_end; });
  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();
}
