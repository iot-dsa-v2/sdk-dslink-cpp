#include "dsa_common.h"
#include "core/session.h"
#include "network/ws/ws_client_connection.h"
#include "util/app.h"
#include "util/enable_shared.h"
#include "web_server/web_server.h"

#include "../async_test.h"
#include "../test_config.h"

#include "gtest/gtest.h"

using namespace dsa;

TEST(WebServerTest, basic_flow) {
  auto app = make_shared_<App>();

// auto web_server = make_shared_<WebServer>(&app, cert);

  // server
  auto web_server = std::make_shared<WebServer>(*app);
  web_server->start();

  // client
  const string_ dsid_prefix = "ws_";
  const string_ ws_host = "127.0.0.1";
  uint16_t ws_port = 8080;
  TestConfig test_config(app, false);
  LinkStrandRef link_strand(std::move(test_config.strand));

  // Simplified websocket client
  auto client = make_shared_<WsClientConnection>(link_strand, dsid_prefix, ws_host,
                                           ws_port);
  client->set_session(make_ref_<Session>(link_strand, ""));
  client->connect(60);

  ASYNC_EXPECT_TRUE(500, *link_strand, [&]() {
    if (client->session()->is_connected()) {
      return true;
    }
  });

  app->close();

  //  WAIT_EXPECT_TRUE(500, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  test_config.destroy();
  app->wait();

    /*
  web_server->listen(port = 80);
  web_server->listen_secure(port=443);


  web_server->add_ws_handler(.... [](){
  new WsConnection();
    */
}

