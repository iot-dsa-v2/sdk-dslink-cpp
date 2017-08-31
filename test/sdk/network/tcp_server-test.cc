#include "dsa/network.h"

#include <atomic>

#include "network/tcp/tcp_client.h"
#include "network/tcp/tcp_server.h"

#include "module/default_modules.h"

#include "gtest/gtest.h"

using namespace dsa;

TEST(TcpServerTest, SingleStrand) {
  App app("SingleStrandTest");

  WrapperConfig config;
  config.tcp_host = "127.0.0.1";
  config.tcp_port = 8092;
  config.strand = make_ref_<DefaultModules>(app);
  config.strand->logger().level = Logger::WARNING;

  app.async_start(10);

  auto tcp_server = make_shared_<TcpServer>(config);
  tcp_server->start();

  const uint32_t NUM_CLIENT = 2;

  std::vector<shared_ptr_<TcpClient>> clients;
  for (unsigned int i = 0; i < NUM_CLIENT; ++i) {
    shared_ptr_<TcpClient> tcp_client(new TcpClient(config));
    tcp_client->connect();
    clients.push_back(std::move(tcp_client));
  }

  std::atomic_bool all_connected{false};
  while (!all_connected) {
    (*config.strand)()->dispatch([&]() {
      bool result = true;
      for (auto& client : clients) {
        if (!client->get_session().is_connected()) {
          result = false;
          break;
        }
      }
      all_connected = result;
    });
    app.sleep(50);
  }

  Server::close_in_strand(tcp_server);
  for (unsigned int i = 0; i < NUM_CLIENT; ++i) {
    Client::close_in_strand(clients[i]);
  }

  app.close();
  for (int i = 0; i < 10; ++i) {
    app.sleep(50);
    if (app.is_stopped()){
      break;
    }
  }

  EXPECT_TRUE(app.is_stopped());

  if (!app.is_stopped()){
    app.force_stop();
  }

  app.wait();
}

TEST(TcpServerTest, MultiStrand) {
  App app("MultiStrandTest");

  WrapperConfig server_config;
  server_config.tcp_host = "127.0.0.1";
  server_config.tcp_port = 8092;
  server_config.strand = make_ref_<DefaultModules>(app);
  server_config.strand->logger().level = Logger::WARNING;

  app.async_start(10);

  //  auto tcp_server(new TcpServer(server_config));
  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  WrapperConfig client_config;
  client_config.tcp_host = "127.0.0.1";
  client_config.tcp_port = 8092;
  client_config.strand = make_ref_<DefaultModules>(app);
  client_config.strand->logger().level = Logger::WARNING;

  const uint32_t NUM_CLIENT = 2;

  std::vector<shared_ptr_<TcpClient>> clients;
  for (unsigned int i = 0; i < NUM_CLIENT; ++i) {
    shared_ptr_<TcpClient> tcp_client(new TcpClient(client_config));
    tcp_client->connect();
    clients.push_back(std::move(tcp_client));
  }

  std::atomic_bool all_connected{false};
  while (!all_connected) {
    (*client_config.strand)()->dispatch([&]() {
      bool result = true;
      for (auto& client : clients) {
        if (!client->get_session().is_connected()) {
          result = false;
          break;
        }
      }
      all_connected = result;
    });
    app.sleep(50);
  }

  Server::close_in_strand(tcp_server);
  for (unsigned int i = 0; i < NUM_CLIENT; ++i) {
    Client::close_in_strand(clients[i]);
  }

  app.close();
  for (int i = 0; i < 10; ++i) {
    app.sleep(50);
    if (app.is_stopped()){
      break;
    }
  }

  EXPECT_TRUE(app.is_stopped());

  if (!app.is_stopped()){
    app.force_stop();
  }

  app.wait();
}
