#include "broker_dslink_util.h"

using BrokerDsLinkTest = SetUpBase;

TEST_F(BrokerDsLinkTest, ClientRemoveTest) {
  Storage::get_config_bucket().remove_all();

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
      broker_dslink_test::create_dslink(app, port, "Test1", false, protocol());
  auto link_2 =
      broker_dslink_test::create_dslink(app, port, "Test2", true, protocol());

  int downstream_listed = 0, clients_listed = 0;
  bool link1_connected = false, allow_all_set = false;
  link_1->connect([&](const shared_ptr_<Connection> connection,
                      ref_<DsLinkRequester> link_req) {
    link1_connected = true;

    // set request to change value
    auto set_request = make_ref_<SetRequestMessage>();
    set_request->set_value(Var(false));
    set_request->set_target_path("Sys/Clients/Allow_All");

    // send set request
    link_req->set(
        [&, link_req](IncomingSetStream &stream,
                      ref_<const SetResponseMessage> &&msg) {
          EXPECT_TRUE(msg->get_status() == Status::OK ||
                      msg->get_status() == Status::DONE);
          allow_all_set = true;

          link_req->list("Downstream", [&, link_req](
                                           IncomingListCache &cache,
                                           const std::vector<string_> &str) {
            auto map = cache.get_map();
            downstream_listed++;
            if (downstream_listed == 1) {
              EXPECT_TRUE(map.find("Test1") != map.end());
              EXPECT_TRUE(map.find("Test2") != map.end());

              link_req->list(
                  "Sys/Clients",
                  [&, link_req](IncomingListCache &cache,
                                const std::vector<string_> &str) {
                    auto map = cache.get_map();
                    bool test1_client = false;
                    string_ test2_id;
                    for (auto &items : map) {
                      if (items.first.substr(0, 5) == "Test1")
                        test1_client = true;
                      else if (items.first.substr(0, 5) == "Test2") {
                        test2_id = items.first;
                      }
                    }
                    clients_listed++;
                    if (clients_listed == 1) {
                      EXPECT_TRUE(test1_client && !test2_id.empty());
                      auto invoke_request = make_ref_<InvokeRequestMessage>();
                      invoke_request->set_target_path("Sys/Clients/" +
                                                      test2_id + "/Remove");

                      link_req->invoke(
                          [&](IncomingInvokeStream &stream,
                              ref_<const InvokeResponseMessage> &&msg) {
                            auto response = std::move(msg);
                            EXPECT_TRUE(response != nullptr);
                            stream.close();
                          },
                          copy_ref_(invoke_request));
                    } else {
                      EXPECT_TRUE(test1_client && test2_id.empty());
                      cache.close();
                    }
                  });

            } else {
              EXPECT_TRUE(map.find("Test1") != map.end());
              EXPECT_TRUE(map.find("Test2") == map.end());
              cache.close();
            }
          });

        },
        std::move(set_request));

  });
  WAIT_EXPECT_TRUE(1000, [&]() -> bool {
    return (link1_connected && (downstream_listed >= 2) &&
            (clients_listed >= 2) && allow_all_set);
  });

  link_1->strand->post([link_1]() { link_1->destroy(); });
  link_2->strand->post([link_2]() { link_2->destroy(); });
  broker->strand->post([broker]() { broker->destroy(); });
  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();

  Storage::get_config_bucket().remove_all();
}

TEST_F(BrokerDsLinkTest, ClientPathTest) {
  Storage::get_config_bucket().remove_all();

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
      broker_dslink_test::create_dslink(app, port, "Test1", false, protocol());

  int downstream_listed = 0;
  bool link1_connected = false, test_end = false;
  link_1->connect([&](const shared_ptr_<Connection> connection,
                      ref_<DsLinkRequester> link_req) {
    link1_connected = true;

    link_req->list(
        "Downstream", [&, link_req](IncomingListCache &cache,
                                    const std::vector<string_> &str) {
          auto map = cache.get_map();
          downstream_listed++;
          if (downstream_listed == 1) {
            EXPECT_TRUE(map.find("Test1") != map.end());

            link_req->list("Sys/Clients", [&, link_req](
                                              IncomingListCache &cache,
                                              const std::vector<string_> &str) {
              auto map = cache.get_map();
              string_ test1_id;
              for (auto &items : map) {
                if (items.first.substr(0, 5) == "Test1") {
                  test1_id = items.first;
                }
              }
              EXPECT_TRUE(!test1_id.empty());

              // change path
              auto set_request = make_ref_<SetRequestMessage>();
              set_request->set_value(Var("Downstream/Test_Changed"));
              set_request->set_target_path("Sys/Clients/" + test1_id + "/Path");
              link_req->set(
                  [&, link_req](IncomingSetStream &stream,
                                ref_<const SetResponseMessage> &&msg) {
                    EXPECT_EQ(msg->get_status(), Status::DONE);

                  },
                  std::move(set_request));
              cache.close();

            });

          } else {
            EXPECT_TRUE(map.find("Test1") == map.end());
            EXPECT_TRUE(map.find("Test_Changed") != map.end());
            cache.close();
            test_end = true;
          }
        });

  });
  WAIT_EXPECT_TRUE(1000, [&]() -> bool {
    return (link1_connected && (downstream_listed >= 2) && test_end);
  });

  link_1->strand->post([link_1]() { link_1->destroy(); });
  broker->strand->post([broker]() { broker->destroy(); });
  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();

  Storage::get_config_bucket().remove_all();
}

bool get_connected(const shared_ptr_<Connection> &connection) {
  if (!connection) {
    return false;
  } else {
    auto session = connection->session();
    if (!session) {
      return false;
    } else {
      return session->is_connected();
    }
  }
}

TEST_F(BrokerDsLinkTest, QuarantineTest) {
  Storage::get_config_bucket().remove_all();

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
      broker_dslink_test::create_dslink(app, port, "Test1", false, protocol());
  auto link_2 =
      broker_dslink_test::create_dslink(app, port, "Test2", false, protocol());

  int downstream_listed = 0, quarantine_listed = 0;
  bool link1_connected = false, link2_connected = false;
  link_1->connect([&](const shared_ptr_<Connection> connection,
                      ref_<DsLinkRequester> link_req) {
    link1_connected = get_connected(connection);
    EXPECT_TRUE(link1_connected);

    // set request to change value
    auto set_request = make_ref_<SetRequestMessage>();
    set_request->set_value(Var(false));
    set_request->set_target_path("Sys/Clients/Allow_All");

    link_req->set(
        [&, link_req](IncomingSetStream &stream,
                      ref_<const SetResponseMessage> &&msg) {
          EXPECT_EQ(msg->get_status(), Status::DONE);

          // set request to change value
          auto set_request = make_ref_<SetRequestMessage>();
          set_request->set_value(Var(true));
          set_request->set_target_path("Sys/Quarantine/Enabled");

          // send set request
          link_req->set(
              [&, link_req](IncomingSetStream &stream,
                            ref_<const SetResponseMessage> &&msg) {
                EXPECT_EQ(msg->get_status(), Status::DONE);

                link_2->connect([&, link1_req = link_req ](
                    const shared_ptr_<Connection> connection2,
                    ref_<DsLinkRequester> link2_req) {
                  link2_connected = get_connected(connection2);
                  EXPECT_TRUE(link2_connected);
                  link1_req->list(
                      "Downstream",
                      [&, link1_req](IncomingListCache &cache,
                                     const std::vector<string_> &str) {
                        auto map = cache.get_map();
                        downstream_listed++;
                        if (downstream_listed == 1) {
                          EXPECT_TRUE(map.find("Test2") == map.end());
                        } else {
                          EXPECT_TRUE(map.find("Test2") != map.end());
                        }
                      });
                  link1_req->list(
                      "Sys/Quarantine",
                      [&, link1_req](IncomingListCache &cache,
                                     const std::vector<string_> &str) {
                        auto map = cache.get_map();
                        quarantine_listed++;
                        string_ test2_id;
                        for (auto &items : map) {
                          if (items.first.substr(0, 5) == "Test2") {
                            test2_id = items.first;
                          }
                        }
                        if (quarantine_listed == 1) {
                          EXPECT_TRUE(!test2_id.empty());
                          // invoke authorize
                          auto invoke_request =
                              make_ref_<InvokeRequestMessage>();
                          invoke_request->set_target_path(
                              "Sys/Quarantine/" + test2_id + "/Authorize");
                          invoke_request->set_value(
                              Var({{"Path", Var("Downstream/Test2")}}));

                          link1_req->invoke(
                              [&](IncomingInvokeStream &stream,
                                  ref_<const InvokeResponseMessage> &&msg) {
                                auto response = std::move(msg);
                                EXPECT_EQ(response->get_status(),
                                          Status::DONE);
                                EXPECT_TRUE(response != nullptr);
                                stream.close();
                              },
                              copy_ref_(invoke_request));
                        } else {
                          EXPECT_TRUE(test2_id.empty());
                        }
                      });

                });

              },
              std::move(set_request));
        },
        std::move(set_request));

  });
  WAIT_EXPECT_TRUE(3000, [&]() -> bool {
    return (link1_connected && link2_connected && (downstream_listed >= 2) &&
            (quarantine_listed >= 2));
  });

  link_1->strand->post([link_1]() { link_1->destroy(); });
  link_2->strand->post([link_2]() { link_2->destroy(); });
  broker->strand->post([broker]() { broker->destroy(); });
  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();

  Storage::get_config_bucket().remove_all();
}

TEST_F(BrokerDsLinkTest, IdenticalClientTest) {
  Storage::get_config_bucket().remove_all();

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
      broker_dslink_test::create_dslink(app, port, "Test1", false, protocol());
  auto link_2 =
      broker_dslink_test::create_dslink(app, port, "Test1", false, protocol());

  bool link1_connected = false;
  bool link2_connected = false;
  link_1->connect([&](const shared_ptr_<Connection> connection,
                      ref_<DsLinkRequester> link_req) {
    link1_connected |= get_connected(connection);

    if (link1_connected) {
      link_2->connect([&](const shared_ptr_<Connection> connection,
                          ref_<DsLinkRequester> link_req) {
        link2_connected |= get_connected(connection);
      });
    }

  });
  WAIT_EXPECT_TRUE(
      1000, [&]() -> bool { return link1_connected && link2_connected; });

  link_1->strand->post([link_1]() { link_1->destroy(); });
  link_2->strand->post([link_2]() { link_2->destroy(); });
  broker->strand->post([broker]() { broker->destroy(); });
  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();

  Storage::get_config_bucket().remove_all();
}