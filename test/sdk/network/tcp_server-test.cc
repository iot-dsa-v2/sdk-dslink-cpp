#include "dsa/network.h"

#include "network/tcp/tcp_client.h"
#include "network/tcp/tcp_server.h"

#include "module/default_modules.h"

#include "gtest/gtest.h"

#include <iostream>

using namespace dsa;

TEST(TcpServerTest, SingleStrand) {
  App app("SingleStrandTest");
  ECDH ecdh;

  app.async_start(10);

  DefaultModules default_modules(app);
  Config server_config = default_modules.get_config();

  server_config.tcp_host = "127.0.0.1";
  server_config.tcp_port = 8092;

  Config client_config = server_config;

  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  std::vector<shared_ptr_<TcpClient>> clients;
  for (unsigned int i = 0; i < 2; ++i) {
    shared_ptr_<TcpClient> tcp_client(
        new TcpClient(client_config));
    tcp_client->connect();
    clients.push_back(std::move(tcp_client));
  }

  app.sleep(2000);

  tcp_server->close();
  for (unsigned int i = 0; i < 2; ++i) {
    clients[i]->close();
  }

  app.close();

  app.wait();
}

TEST(TcpServerTest, MultiStrand) {
  App app("MultiStrandTest");

  app.async_start(10);

  DefaultModules default_modules(app);
  Config server_config = default_modules.get_config();

  server_config.tcp_host = "127.0.0.1";
  server_config.tcp_port = 8092;

  Config client_config = default_modules.get_config();

  ServerPtr tcp_server(new TcpServer(server_config));
  tcp_server->start();

  std::vector<shared_ptr_<TcpClient>> clients;
  for (unsigned int i = 0; i < 2; ++i) {
    shared_ptr_<TcpClient> tcp_client(
        new TcpClient(client_config));
    tcp_client->connect();
    clients.push_back(std::move(tcp_client));
  }

  app.sleep(2000);

  tcp_server->close();
  for (unsigned int i = 0; i < 2; ++i) {
    clients[i]->close();
  }

  app.close();

  app.wait();
}
