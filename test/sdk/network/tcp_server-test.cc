#include "dsa/network.h"
#include "gtest/gtest.h"

#include <iostream>

using namespace dsa;

TEST(TcpServerTest, OneClient) {
  shared_ptr_<App> app;
  ASSERT_NO_FATAL_FAILURE(app.reset(new App("Test")));

  app->async_start(2);

  Server::Config server_config("/test/path", 8000);
  Client::Config client_config("127.0.0.1", 8000);

  std::shared_ptr<Server> tcp_server(app->new_server(Server::TCP, server_config));
  tcp_server->start();

  app->sleep(500);

//  std::shared_ptr<Client> tcp_client(app->new_client(Client::TCP, client_config));
//  tcp_client->connect();

  app->sleep(1000);

  app->graceful_stop(1000);

  app->wait();
}

TEST(TcpServerTest, MultipleClients) {
  shared_ptr_<App> app;
  ASSERT_NO_FATAL_FAILURE(app.reset(new App("Test")));

  app->async_start(10);

  Server::Config server_config("/test/path", 8081);
  Client::Config client_config("127.0.0.1", 8081);

  ServerPtr tcp_server(app->new_server(Server::TCP, server_config));
  tcp_server->start();

  app->sleep(1000);

  for (unsigned int i = 0; i < 2; ++i) {
    ClientPtr tcp_client(app->new_client(Client::TCP, client_config));
    tcp_client->connect();
  }

  app->sleep(1000);

  app->graceful_stop(1000);
  app->wait();
}
