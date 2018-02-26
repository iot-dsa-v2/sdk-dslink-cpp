#include "dsa/network.h"
#include "dsa/responder.h"
#include "dsa/stream.h"

#include <gtest/gtest.h>
#include "../test/sdk/async_test.h"
#include "../test/sdk/test_config.h"
#include "network/tcp/tcp_server.h"

#include "module/default/console_logger.h"
#include "module/default/simple_security.h"
#include "module/default/simple_session_manager.h"

#include "web_server/http_request.h"

using namespace dsa;

// TEST(NetworkTest, ReConnect) {
//  auto app = std::make_shared<App>();
//
//  TestConfig server_strand(app);
//
//  server_strand.strand->set_responder_model(
//      make_ref_<NodeModel>(server_strand.strand->get_ref()));
//
//  auto tcp_server = server_strand.create_server();
//  tcp_server->start();
//
//  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();
//
//  auto client = make_ref_<Client>(client_strand);
//  client->connect();
//
//  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
//                    [&]() { return client->get_session().is_connected(); });
//
//  // close everything
//  tcp_server->destroy_in_strand(tcp_server);
//  destroy_client_in_strand(client);
//
//  server_strand.destroy();
//  client_strand.destroy();
//
//  app->close();
//
//  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });
//
//  if (!app->is_stopped()) {
//    app->force_stop();
//  }
//
//  app->wait();
//}

int main() {
  auto app = std::make_shared<App>();
  TestConfig server_strand(app);
  auto web_server = server_strand.create_webserver();

  WebServer::HttpCallback root_cb = [](WebServer &web_server,
                                       HttpRequest &&req) {
    req.redirect_handler("/login", "Redirecting...");
  };
  WebServer::HttpCallback login_cb = [](WebServer &web_server,
                                        HttpRequest &&req) {};
  WebServer::HttpCallback not_found_cb = [](WebServer &web_server,
                                            HttpRequest &&req) {
    req.not_found_handler("Not authorized to access this URL");
    std::cout << "Unauthorized access attempted."
              << std::endl;  // Log it anyway
  };
  WebServer::HttpCallback default_file_server_cb = [](WebServer &web_server,
                                                      HttpRequest &&req) {
    req.file_server_handler("");
  };
  WebServer::HttpCallback authentication_cb = [](WebServer &web_server,
                                                 HttpRequest &&req) {
    req.authentication_handler();
  };

  web_server->add_http_handler("/", std::move(root_cb));
  web_server->add_http_handler("/login", std::move(authentication_cb));
  web_server->add_http_handler("/unauthorized", std::move(not_found_cb));
  web_server->add_http_handler("/default", std::move(default_file_server_cb));
  web_server->start();

  app->wait();
}