#include "core/link_strand.h"
#include "core/config.h"
#include "dsa/network.h"

#include "network/tcp/tcp_client.h"
#include "network/tcp/tcp_server.h"

#include "module/default_modules.h"

#include "gtest/gtest.h"

#include <iostream>

using namespace dsa;

TEST(TcpServerTest, SingleStrand) {
  App app("SingleStrandTest");

  WrapperConfig config;
  config.tcp_host = "127.0.0.1";
  config.tcp_port = 8092;

  app.async_start(10);
  config.strand = make_intrusive_<DefaultModules>(app);

  auto tcp_server = make_shared_<TcpServer>(config);
  tcp_server->start();

  std::vector<shared_ptr_<TcpClient>> clients;
  for (unsigned int i = 0; i < 2; ++i) {
    shared_ptr_<TcpClient> tcp_client(
        new TcpClient(config));
    tcp_client->connect();
    clients.push_back(std::move(tcp_client));
  }

  app.sleep(2000);

  tcp_server->close();
  for (unsigned int i = 0; i < 2; ++i) {
    clients[i]->close();
  }

  app.sleep(500);
  // TODO: check if app has pending jobs

  app.force_stop();

  app.wait();
}
/*
TEST(TcpServerTest, MultiStrand) {
  WrapperConfig config;
  config.tcp_host = "127.0.0.1";
  config.tcp_port = 8092;

  App app("MultiStrandTest");
  app.async_start(10);

  ServerPtr tcp_server(new TcpServer(config));
  tcp_server->start();

  std::vector<shared_ptr_<TcpClient>> clients;
  for (unsigned int i = 0; i < 2; ++i) {
    shared_ptr_<TcpClient> tcp_client(
        new TcpClient(config));
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
}*/
