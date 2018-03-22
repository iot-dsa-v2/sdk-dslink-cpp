#include "broker_dslink_util.h"

using namespace dsa;

using BrokerDsLinkTest = SetUpBase;
const string_ bucket_name("config");
TEST_F(BrokerDsLinkTest, Reconnect) {
  auto app = make_shared_<App>();

  auto broker = broker_dslink_test::create_broker(app);
  broker->run(false);

  int32_t port;

  switch (protocol()) {
    case dsa::ProtocolType::PROT_DSS:
      port = broker->get_active_secure_port();
      break;
    default:
      port = broker->get_active_server_port();
  }

  EXPECT_TRUE(port != 0);

  for (int i = 0; i < 3; i++) {
    // Changing name
    auto changing_name = broker_dslink_test::create_dslink(
        app, port, "Changing_Name_Link" + i, false, protocol());

    bool connected = false;
    changing_name->connect(
        [&](const shared_ptr_<Connection> connection,
            ref_<DsLinkRequester> link_req) { connected = true; });
    WAIT_EXPECT_TRUE(1000, [&]() -> bool { return connected; });
    changing_name->strand->post([&]() { changing_name->destroy(); });

    // Constant name with listing
    auto link_1 =
        broker_dslink_test::create_dslink(app, port, "Test1", false, protocol());

    connected = false;
    link_1->connect([&](const shared_ptr_<Connection> connection,
                        ref_<DsLinkRequester> link_req) { connected = true; });
    WAIT_EXPECT_TRUE(1000, [&]() -> bool { return connected; });

    // Constant name main link
    auto main_link = broker_dslink_test::create_dslink(app, port, "Main_Link",
                                                       false, protocol());

    bool flag1 = false, flag2 = false;
    //    bool test_done = false;
    main_link->connect([&](const shared_ptr_<Connection> connection,
                           ref_<DsLinkRequester> link_req) {
      // 2. CHECK IF CONNECTION IS OK
      auto list_path = "Downstream/Test1";

      link_req->list(list_path, [&](IncomingListCache &cache,
                                   const std::vector<string_> &str) {
        auto status = cache.get_status();
        if (status == MessageStatus::OK) {
          flag1 = true;

          // 3. CHECK DISCONNECT
          link_1->strand->post([&]() { link_1->destroy(); });
        }
        if ((status == MessageStatus::NOT_AVAILABLE) && flag1) {
          flag2 = true;
          cache.close();
        }
      });
    });
    WAIT_EXPECT_TRUE(3000, [&]() -> bool { return flag1 && flag2; });

    main_link->strand->post([main_link]() { main_link->destroy(); });

    WAIT(500);
  }

  broker->strand->post([&]() { broker->destroy(); });

  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();
}

TEST_F(BrokerDsLinkTest, NotAvailableStep3) {
  auto broker = broker_dslink_test::create_broker();
  shared_ptr_<App> &app = broker->get_app();
  broker->run(false);

  int32_t port;

  switch (protocol()) {
    case dsa::ProtocolType::PROT_DSS:
      port = broker->get_active_secure_port();
      break;
    default:
      port = broker->get_active_server_port();
  }

  EXPECT_TRUE(port != 0);

  // broker->strand->logger().level = Logger::ALL___;

  auto link_1 =
      broker_dslink_test::create_dslink(app, port, "Test_1", false, protocol());
  // link_1->strand->logger().level = Logger::ALL___;
  auto link_2 =
      broker_dslink_test::create_dslink(app, port, "Test_2", false, protocol());
  // link_2->strand->logger().level = Logger::ALL___;

  int step = 0;
  // when list on downstream/test1 it should have a metadata for test1's dsid
  auto unavailable_child_list = [&](const shared_ptr_<Connection> &connection,
                                    ref_<DsLinkRequester> link_req) {
    link_req->list("Downstream/Test_2", [&](IncomingListCache &cache,
                                           const std::vector<string_>) {
      step++;
      switch (step) {
        case 1: {
          // step 1, connect client 2
          // std::cout<<"Step 1"<<std::endl;
          EXPECT_EQ(cache.get_status(), MessageStatus::NOT_AVAILABLE);
          link_2->connect();
          break;
        }
        case 2: {
          // step 2, disconnect client 2
          // std::cout<<"Step 2"<<std::endl;
          EXPECT_EQ(cache.get_status(), MessageStatus::OK);
          link_2->strand->post([link_2]() { link_2->destroy(); });
          break;
        }

        default: {  //   case 3:{
          // step 3, end test
          // std::cout<<"Step 3"<<std::endl;
          EXPECT_EQ(cache.get_status(), MessageStatus::NOT_AVAILABLE);
          link_1->strand->post([link_1]() { link_1->destroy(); });
          broker->strand->post([broker]() { broker->destroy(); });
        }
      }

    });
  };

  link_1->connect(std::move(unavailable_child_list));

  broker->wait();
  EXPECT_TRUE(broker->is_destroyed());
}

TEST_F(BrokerDsLinkTest, StopTest) {

  std::string master_token = "12345678901234567890123456789012";
  SimpleSafeStorageBucket storage_bucket(bucket_name, nullptr,"");
  string_to_storage(master_token, default_master_token_path, storage_bucket);

  auto broker = broker_dslink_test::create_broker();
  shared_ptr_<App> &app = broker->get_app();
  broker->run(false);

  int32_t port;

  switch (protocol()) {
    case dsa::ProtocolType::PROT_DSS:
      port = broker->get_active_secure_port();
      break;
    default:
      port = broker->get_active_server_port();
  }

  EXPECT_TRUE(port != 0);

  bool connected = false;
  auto link_1 =
      broker_dslink_test::create_dslink(app, port, "Test_1", false, protocol());
  link_1->connect([&](const shared_ptr_<Connection> connection,
                      ref_<DsLinkRequester> link_req) { connected = true; });
  WAIT_EXPECT_TRUE(500, [&]() -> bool { return connected; });
  connected = false;
  bool invoked = false;
  auto link_2 =
      broker_dslink_test::create_dslink(app, port, "Test_2", false, protocol());
  link_2->connect([&](const shared_ptr_<Connection> connection,
                      ref_<DsLinkRequester> link_req) {
    ref_<InvokeRequestMessage> invoke_req = make_ref_<InvokeRequestMessage>();
    invoke_req->set_value(Var(master_token));
    invoke_req->set_target_path("Downstream/Test_1/Sys/Stop");

    link_req->invoke(
        [&](IncomingInvokeStream &stream,
            ref_<const InvokeResponseMessage> &&msg) {
          // no response here!!!
          broker->strand->post([broker]() { broker->destroy(); });
          link_2->strand->post([link_2]() { link_2->destroy(); });
          invoked = true;
        },
        std::move(invoke_req));
    connected = true;
  });
  WAIT_EXPECT_TRUE(500, [&]() -> bool { return connected && invoked; });
  WAIT_EXPECT_TRUE(2000, [&]() -> bool { return link_1->is_destroyed(); });
  storage_bucket.remove(default_master_token_path);
  broker->wait();
  EXPECT_TRUE(broker->is_destroyed());
}

TEST_F(BrokerDsLinkTest, SysListWithCloseToken) {
  std::string master_token = generate_random_string(32);
  SimpleSafeStorageBucket storage_bucket(bucket_name, nullptr,"");
  string_to_storage(master_token, default_master_token_path, storage_bucket);

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
      broker_dslink_test::create_dslink(app, port, "Test_1", false, protocol());

  bool listed = false;
  link_1->connect(
      [&](const shared_ptr_<Connection> connection, ref_<DsLinkRequester> link_req) {
        link_req->list(
            "Sys", [&](IncomingListCache &cache, const std::vector<string_>) {
              VarMap vm = cache.get_map();
              EXPECT_TRUE(vm["Stop"].get_type() != Var::NUL);
              listed = true;
            });
      });

  WAIT_EXPECT_TRUE(1000, [&]() { return listed; });

  // Closing

  link_1->strand->post([&]() { link_1->destroy(); });

  broker->strand->post([&]() { broker->destroy(); });
  storage_bucket.remove(default_master_token_path);

  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();
}

TEST_F(BrokerDsLinkTest, SysListWithoutCloseToken) {
  // we force to not have close
  std::remove("config/.master_token");

  auto app = make_shared_<App>();

  auto broker = broker_dslink_test::create_broker(app);
  broker->run(false);

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
      broker_dslink_test::create_dslink(app, port, "Test_1", false, protocol());

  bool listed = false;
  link_1->connect(
      [&](const shared_ptr_<Connection> connection, ref_<DsLinkRequester> link_req) {
        link_req->list(
            "Sys", [&](IncomingListCache &cache, const std::vector<string_>) {
              VarMap vm = cache.get_map();
              EXPECT_TRUE(vm["Stop"].get_type() == Var::NUL);
              listed = true;
            });
      });

  WAIT_EXPECT_TRUE(1000, [&]() { return listed; });

  // Closing
  link_1->strand->post([&]() { link_1->destroy(); });

  broker->strand->post([&]() { broker->destroy(); });

  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();
}
