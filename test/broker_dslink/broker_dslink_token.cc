#include "broker_dslink_util.h"

using BrokerDsLinkTest = SetUpBase;

TEST_F(BrokerDsLinkTest, TokenTest) {
  // Storage::get_config_bucket().remove_all();

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

  auto link_1 =
      broker_dslink_test::create_dslink(app, port, "Test1", false, protocol());
  auto link_2 =
      broker_dslink_test::create_dslink(app, port, "Test2", false, protocol());
  auto link_3 =
      broker_dslink_test::create_dslink(app, port, "Test3", false, protocol());

  bool allow_all_set = false;
  string_ token;
  bool link_2_connected = false;
  int link_3_connected = 0;
  link_1->connect([&](const shared_ptr_<Connection> connection,
                      ref_<DsLinkRequester> link_req) {

    // send set request
    link_req->set(
        [&, link_req](IncomingSetStream &stream,
                      ref_<const SetResponseMessage> &&msg) {
          EXPECT_TRUE(msg->get_status() == Status::DONE);
          allow_all_set = true;
        },
        make_ref_<SetRequestMessage>("Sys/Clients/Allow_All", Var(false)));

    link_req->invoke(
        [&, link_req](IncomingInvokeStream &stream,
                      ref_<const InvokeResponseMessage> &&msg) {
          EXPECT_TRUE(msg->get_status() == Status::DONE);
          Var v = msg->get_value();
          EXPECT_TRUE(v.is_map());
          token = v["Token"].to_string();
          EXPECT_FALSE(token.empty());
        },
        make_ref_<InvokeRequestMessage>("Sys/Tokens/Add",
                                        Var({{"Count", Var(1)}})));

  });
  WAIT_EXPECT_TRUE(1000,
                   [&]() -> bool { return allow_all_set && !token.empty(); });

  // connect link 2 without token
  link_2->connect(
      [&](const shared_ptr_<Connection> connection,
          ref_<DsLinkRequester> link_req) { link_2_connected = true; });
  // connect link 3 with token,
  link_3->client_token = token;
  link_3->connect(
      [&](const shared_ptr_<Connection> connection,
          ref_<DsLinkRequester> link_req) {
        EXPECT_FALSE(link_2_connected);
        link_3_connected++;
        if (link_3_connected == 1) {
          // disconnect it once and see if it still reconnect
          connection->destroy();
        }
      },
      Client::EVERY_CONNECTION);

  WAIT_EXPECT_TRUE(1000, [&]() -> bool {
    return link_3_connected == 2 && !link_2_connected;
  });

  link_1->strand->post([link_1]() { link_1->destroy(); });
  link_2->strand->post([link_2]() { link_2->destroy(); });
  link_3->strand->post([link_3]() { link_3->destroy(); });
  broker->strand->post([broker]() { broker->destroy(); });
  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();
}
