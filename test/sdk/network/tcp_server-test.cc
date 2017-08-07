#include "dsa/network.h"

#include "network/tcp_connection.h"
#include "network/tcp_server.h"

#include "gtest/gtest.h"

#include <iostream>

using namespace dsa;

TEST(TcpServerTest, OneClient) {
  shared_ptr_<App> app;
  ASSERT_NO_FATAL_FAILURE(app.reset(new App("Test")));

  app->async_start(2);

  Config server_config;
  server_config.tcp_host = "127.0.0.1";
  server_config.tcp_port = 8091;
  //  Client::Config client_config("127.0.0.1", 8000);

  std::shared_ptr<Server> tcp_server(new TcpServer(*app, server_config));
  tcp_server->start();

  app->sleep(500);

  //  std::shared_ptr<Client> tcp_client(app->new_client(Client::TCP,
  //  client_config)); tcp_client->connect();

  app->sleep(1000);

  tcp_server->close();

  app->close();

  app->wait();
}

TEST(TcpServerTest, MultipleClients) {
  shared_ptr_<App> app;
  ASSERT_NO_FATAL_FAILURE(app.reset(new App("Test")));

  app->async_start(10);

  Config server_config;
  server_config.tcp_host = "127.0.0.1";
  server_config.tcp_port = 8092;

  Config client_config = server_config;

  ServerPtr tcp_server(new TcpServer(*app, server_config));
  tcp_server->start();

  app->sleep(1000);

  std::vector<shared_ptr_<TcpClientConnection>> clients;
  for (unsigned int i = 0; i < 2; ++i) {
    shared_ptr_<TcpClientConnection> tcp_client(
        new TcpClientConnection(*app, client_config));
    tcp_client->connect();
    clients.push_back(std::move(tcp_client));
  }

  app->sleep(1000);

  tcp_server->close();
  for (unsigned int i = 0; i < 2; ++i) {
    clients[i]->close();
  }

  app->close();

  app->wait();
}
