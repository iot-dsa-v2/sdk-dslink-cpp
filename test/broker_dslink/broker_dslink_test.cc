#include "broker_dslink_util.h"

using namespace dsa;

TEST(BROKER_DSLINK_TEST, Reconnect) {
  auto app = make_shared_<App>();

  auto broker = broker_dslink_test::create_broker(app);
  broker->run(false);
  WAIT_EXPECT_TRUE(500,
                   [&]() { return broker->get_active_server_port() != 0; });

  for (int i = 0; i < 3; i++) {
    // Constant name main link
    auto main_link = broker_dslink_test::create_dslink(
        app, broker->get_active_server_port(), "main_link", true);

    // Changing name
    auto changing_name = broker_dslink_test::create_dslink(
        app, broker->get_active_server_port(), "changing_name_link" + i, true);
    changing_name->strand->post([&]() { changing_name->destroy(); });

    // Constant name with listing
    auto link_1 = broker_dslink_test::create_dslink(
        app, broker->get_active_server_port(), "test1", true);

    // 2. CHECK IF CONNECTION IS OK
    auto list_path = "downstream/test1";

    MessageStatus status = MessageStatus::ALIAS_LOOP;
    auto downstream_list = main_link->list(
        list_path,
        [&](IncomingListCache &cache, const std::vector<string_> &str) {
          status = cache.get_status();
        });

    WAIT_EXPECT_TRUE(1000,
                     [&]() -> bool { return status == MessageStatus::OK; });
    downstream_list->destroy();

    // 3. CHECK DISCONNECT
    link_1->strand->post([&]() { link_1->destroy(); });
    WAIT(500);

    status = MessageStatus::ALIAS_LOOP;
    downstream_list = main_link->list(
        list_path,
        [&](IncomingListCache &cache, const std::vector<string_> &str) {
          status = cache.get_status();
        });

    WAIT_EXPECT_TRUE(
        1000, [&]() -> bool { return status == MessageStatus::NOT_AVAILABLE; });
    downstream_list->destroy();

    main_link->strand->post([main_link]() { main_link->destroy(); });

    WAIT(500);
  }

  broker->strand->post([&]() { broker->destroy(); });

  app->close();

  WAIT_EXPECT_TRUE(500, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();
}

TEST(BROKER_DSLINK_TEST, NOT_AVAILABLE_3_STEP) {
  auto broker = broker_dslink_test::create_broker();
  shared_ptr_<App> &app = broker->get_app();
  broker->run(false);
  WAIT_EXPECT_TRUE(500,
                   [&]() { return broker->get_active_server_port() != 0; });

  // broker->strand->logger().level = Logger::ALL___;

  auto link_1 = broker_dslink_test::create_dslink(
      app, broker->get_active_server_port(), "test_1");
  // link_1->strand->logger().level = Logger::ALL___;
  auto link_2 = broker_dslink_test::create_dslink(
      app, broker->get_active_server_port(), "test_2");
  // link_2->strand->logger().level = Logger::ALL___;

  int step = 0;
  // when list on downstream/test1 it should have a metadata for test1's dsid
  auto unavailable_child_list = [&](const shared_ptr_<Connection> &connection) {
    link_1->list("downstream/test_2", [&](IncomingListCache &cache,
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

TEST(BROKER_DSLINK_TEST, STOP_TEST) {
  std::string close_token = "12345678901234567890123456789012";
  string_to_file(close_token, ".close_token");

  auto broker = broker_dslink_test::create_broker();
  shared_ptr_<App> &app = broker->get_app();
  broker->run(false);
  WAIT_EXPECT_TRUE(500,
                   [&]() { return broker->get_active_server_port() != 0; });

  auto link_1 = broker_dslink_test::create_dslink(
      app, broker->get_active_server_port(), "test_1",true);
  auto link_2 = broker_dslink_test::create_dslink(
      app, broker->get_active_server_port(), "test_2",true);

  ref_<InvokeRequestMessage> invoke_req = make_ref_<InvokeRequestMessage>();
  invoke_req->set_value(Var(close_token));
  invoke_req->set_target_path("downstream/test_1/sys/stop");

  link_2->invoke(
      [&](IncomingInvokeStream &stream,
          ref_<const InvokeResponseMessage> &&msg) {
        // no response here!!!
        std::cout << "Close Invoked" << std::endl;
        broker->strand->post([broker]() { broker->destroy(); });
        link_2->strand->post([link_2]() { link_2->destroy(); });
      },
      std::move(invoke_req));

  WAIT_EXPECT_TRUE(2000,
                   [&]() { return link_1->is_destroyed(); });

  broker->wait();
  EXPECT_TRUE(broker->is_destroyed());
}

TEST(BROKER_DSLINK_TEST, SYS_LIST_WITH_CLOSE_TOKEN) {
  std::string close_token = "12345678901234567890123456789012";
  string_to_file(close_token, ".close_token");

  auto app = make_shared_<App>();

  auto broker = broker_dslink_test::create_broker(app);
  broker->run();
  WAIT_EXPECT_TRUE(500,
                   [&]() { return broker->get_active_server_port() != 0; });

  auto link_1 = broker_dslink_test::create_dslink(
      app, broker->get_active_server_port(), "test_1", true);

  VarMap vm;
  bool is_invoked = false;
  link_1->list("sys",
               [&](IncomingListCache &cache, const std::vector<string_>) {
                 vm = cache.get_map();
                 is_invoked = true;
               });

  WAIT_EXPECT_TRUE(1000, [&]() { return is_invoked; });
  EXPECT_TRUE(vm["stop"].get_type() != Var::NUL);

  // Closing

  link_1->strand->post([&]() { link_1->destroy(); });

  broker->strand->post([&]() { broker->destroy(); });

  app->close();

  WAIT_EXPECT_TRUE(500, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();
}

TEST(BROKER_DSLINK_TEST, SYS_LIST_WITHOUT_CLOSE_TOKEN) {
  // we force to not have close
  std::remove(".close_token");

  auto app = make_shared_<App>();

  auto broker = broker_dslink_test::create_broker(app);
  broker->run(false);
  WAIT_EXPECT_TRUE(500,
                   [&]() { return broker->get_active_server_port() != 0; });

  auto link_1 = broker_dslink_test::create_dslink(
      app, broker->get_active_server_port(), "test_1", true);

  VarMap vm;
  bool is_invoked = false;
  link_1->list("sys",
               [&](IncomingListCache &cache, const std::vector<string_>) {
                 vm = cache.get_map();
                 is_invoked = true;
               });

  WAIT_EXPECT_TRUE(1000, [&]() { return is_invoked; });
  EXPECT_TRUE(vm["stop"].get_type() == Var::NUL);

  // Closing
  link_1->strand->post([&]() { link_1->destroy(); });

  broker->strand->post([&]() { broker->destroy(); });

  app->close();

  WAIT_EXPECT_TRUE(500, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();
}