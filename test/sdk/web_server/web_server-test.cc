#include "dsa_common.h"
#include "network/ws/ws_client_connection.h"
#include "util/app.h"
#include "util/enable_shared.h"
#include "web_server/web_server.h"

#include "../async_test.h"
#include "../test_config.h"

#include "gtest/gtest.h"

using namespace dsa;

TEST(WebServerTest, basic_flow) {
  App app;

// auto web_server = make_shared_<WebServer>(app, cert);

  // server
  auto web_server = make_shared_<WebServer>(app);
  web_server->start();

  // client
  const string_ dsid_prefix = "ws_";
  const string_ ws_host = "127.0.0.1";
  uint16_t ws_port = 0;
  TestConfig test_config(app, false);
  LinkStrandRef link_strand(std::move(test_config.strand));
  make_shared_<WsClientConnection>(link_strand, dsid_prefix, ws_host,
                                          ws_port);

#if 0
  const uint32_t NUM_CLIENT = 2;

  std::vector<ref_<Client>> clients;
  for (unsigned int i = 0; i < NUM_CLIENT; ++i) {
    var tcp_client = make_ref_<client>(config);
    tcp_client->connect();
    clients.push_back(std::move(tcp_client));
  }

  ASYNC_EXPECT_TRUE(500, *config.strand, [&]() {
    for (auto& client : clients) {
      if (!client->get_session().is_connected()) {

      }
    }
    return true;
  });

  // close everything
  tcp_server->destroy_in_strand(tcp_server);
  for (unsigned int i = 0; i < NUM_CLIENT; ++i) {
    destroy_client_in_strand(clients[i]);
  }
#endif

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();

    /*
  web_server->listen(port = 80);
  web_server->listen_secure(port=443);


  web_server->add_ws_handler(.... [](){
  new WsConnection();
    */
}

