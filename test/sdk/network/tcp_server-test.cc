#include "dsa/network.h"
#include "gtest/gtest.h"

#include <iostream>

using namespace dsa;

TEST(TcpServerTest, NewServer) {
  std::shared_ptr<App> app;
  ASSERT_NO_FATAL_FAILURE(app.reset(new App("Test")));

  Server::Config config;
  config.set_port(8080);

  app->add_server(Server::TCP, config);

  Client::Config client_config;
  client_config.set_host("127.0.0.1");
  client_config.set_port(8080);

  ClientPtr client = app->new_client(client_config);
  client->connect();
  app->run();
}
