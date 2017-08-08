#include "dsa/network.h"

#include "network/tcp_client.h"
#include "network/tcp_server.h"

#include "gtest/gtest.h"

#include <iostream>

using namespace dsa;

TEST(TcpServerTest, SingleStrand) {
  shared_ptr_<App> app;
  ECDH ecdh;

  ASSERT_NO_FATAL_FAILURE(app.reset(new App("Test")));

  app->async_start(10);

  Config server_config(&ecdh);
  server_config.tcp_host = "127.0.0.1";
  server_config.tcp_port = 8092;

  Config client_config = server_config;

  auto global_strand = app->new_strand();

  ServerPtr tcp_server(new TcpServer(global_strand, server_config));
  tcp_server->start();

  std::vector<shared_ptr_<TcpClient>> clients;
  for (unsigned int i = 0; i < 2; ++i) {
    shared_ptr_<TcpClient> tcp_client(
        new TcpClient(global_strand, client_config));
    tcp_client->connect();
    clients.push_back(std::move(tcp_client));
  }

  app->sleep(2000);

  tcp_server->close();
  for (unsigned int i = 0; i < 2; ++i) {
    clients[i]->close();
  }

  app->close();

  app->wait();
}

TEST(TcpServerTest, MultiStrand) {
  shared_ptr_<App> app;
  ECDH ecdh;

  ASSERT_NO_FATAL_FAILURE(app.reset(new App("Test")));

  app->async_start(10);

  Config server_config(&ecdh);
  server_config.tcp_host = "127.0.0.1";
  server_config.tcp_port = 8092;

  Config client_config = server_config;

  ServerPtr tcp_server(new TcpServer(app->new_strand(), server_config));
  tcp_server->start();

  std::vector<shared_ptr_<TcpClient>> clients;
  for (unsigned int i = 0; i < 2; ++i) {
    shared_ptr_<TcpClient> tcp_client(
        new TcpClient(app->new_strand(), client_config));
    tcp_client->connect();
    clients.push_back(std::move(tcp_client));
  }

  app->sleep(2000);

  tcp_server->close();
  for (unsigned int i = 0; i < 2; ++i) {
    clients[i]->close();
  }

  app->close();

  app->wait();
}
