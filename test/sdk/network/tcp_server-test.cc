#include "dsa/network.h"
#include "gtest/gtest.h"

#include <iostream>

using namespace dsa;

TEST(TcpServerTest, OneClient) {
  std::shared_ptr<App> app;
  ASSERT_NO_FATAL_FAILURE(app.reset(new App("Test")));

  app->async_start(2);

  Server::Config server_config("/test/path", 8080);
  Client::Config client_config("127.0.0.1", 8080);

  ServerPtr tcp_server = app->new_server(Server::TCP, server_config);
  tcp_server->start();

  ClientPtr tcp_client = app->new_client(Client::TCP, client_config);
  tcp_client->connect();

  app->graceful_stop();

  app->sleep(2000);
  tcp_server->stop();

  app->wait();
}

TEST(TcpServerTest, MultipleClients) {
  std::shared_ptr<App> app;
  ASSERT_NO_FATAL_FAILURE(app.reset(new App("Test")));

  app->async_start(10);

  Server::Config server_config("/test/path", 8081);
  Client::Config client_config("127.0.0.1", 8081);

//  ServerPtr tcp_server = app->new_server(Server::TCP, server_config);
//  tcp_server->start();
//
//  app->sleep(1000);

//  for (unsigned int i = 0; i < 10; ++i) {
//    (app->new_client(Client::TCP, client_config))->connect();
//    app->sleep(1000);
//  }

//  app->sleep(2000);

  app->stop();
  app->wait();
}
