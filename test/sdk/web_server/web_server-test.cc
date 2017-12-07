#include "dsa_common.h"

#include <boost/beast/http.hpp>

#include "network/ws/ws_callback.h"
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
  web_server->listen(8080);
  web_server->start();
  WebServer::WsCallback root_cb = [](
      WebServer& web_server,
      boost::asio::ip::tcp::socket&& socket,
      boost::beast::http::request<boost::beast::http::string_body> req) {
    DsaWsCallback()(web_server.io_service(), std::move(socket), std::move(req));
  };

  web_server->add_ws_handler("/", std::move(root_cb));

  // client
  TestConfig test_config(app, false);
  WrapperStrand config;
  const string_ dsid_prefix = "ws_";
  config.ws_host = "127.0.0.1";
  config.ws_port = 8080;
  config.ws_path = "/";

  config.strand = EditableStrand::make_default(app);
  config.strand->logger().level = test_config.strand->logger().level;
  config.client_connection_maker =
      [
        dsid_prefix = dsid_prefix, ws_host = config.ws_host,
        ws_port = config.ws_port
      ](LinkStrandRef & strand, const string_& previous_session_id,
        int32_t last_ack_id) {
    return make_shared_<WsClientConnection>(strand, dsid_prefix, ws_host,
                                            ws_port);
  };

  ref_<Client> client(new Client(config));
  client->connect();

  ASYNC_EXPECT_TRUE(500, *config.strand, [&]() {
    if (!client->get_session().is_connected()) {
      return false;
    }
    return true;
  });

  app->close();

  // WAIT_EXPECT_TRUE(500, [&]() -> bool { return app->is_stopped(); });

  //  if (!app->is_stopped()) {
  //    app->force_stop();
  //  }

  config.destroy();
  test_config.destroy();
  //  app->wait();
}
