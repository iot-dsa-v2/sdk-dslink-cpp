#include "dsa/message.h"
#include "dsa/network.h"
#include "dsa/responder.h"
#include "dsa/stream.h"

#include <gtest/gtest.h>
#include "../async_test.h"
#include "../test_config.h"

#include "../../broker/util/broker_runner.h"
#include "module/default/console_logger.h"
#include "network/tcp/tcp_server.h"

using namespace dsa;

using DslinkTest = SetUpBase;

TEST_F(DslinkTest, ConnectTest) {
  auto app = std::make_shared<App>();

  TestConfig server_strand(app, false, protocol());

  server_strand.strand->set_responder_model(
      make_ref_<NodeModel>(server_strand.strand->get_ref()));

  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  auto web_server = server_strand.create_webserver();
  web_server->start();

  auto link = server_strand.create_dslink(true);

  int step = 0;
  link->connect(
      [&](const shared_ptr_<Connection> connection, ref_<DsLinkRequester> link_req) {
        ++step;
        switch (step) {
          case 1: {
            // connected
            EXPECT_TRUE(connection != nullptr);
            connection->destroy_in_strand(connection);
            break;
          }
          case 2: {
            // disconencted
            EXPECT_TRUE(connection == nullptr);
            break;
          }
          case 3: {
            // reconnected
            EXPECT_TRUE(connection != nullptr);
            break;
          }
          default: {
            // not expected
            EXPECT_TRUE(false);
          }
        }
      },
      Client::FIRST_CONNECTION | Client::BROKER_INFO_CHANGE |
          Client::EVERY_CONNECTION | Client::DISCONNECTION);

  ASYNC_EXPECT_TRUE(2000, *link->strand, [&]() { return step >= 3; });

  tcp_server->destroy_in_strand(tcp_server);
  web_server->destroy();
  destroy_dslink_in_strand(link);

  ASYNC_EXPECT_TRUE(1000, *link.get()->strand, [&]() -> bool {
    return link->is_destroyed() && link->ref_count() == 1;
  });

  app->close();

  server_strand.destroy();
  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();
}
