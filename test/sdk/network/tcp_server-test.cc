#include "dsa/network.h"

#include <atomic>

#include "core/client.h"
#include "network/tcp/tcp_server.h"

#include "../async_test.h"
#include "../test_config.h"

#include "gtest/gtest.h"

using namespace dsa;

TEST(TcpServerTest, SingleThread) {
  App app(1);

  WrapperConfig config = TestConfig(app).get_client_config(app);

  auto tcp_server = make_shared_<TcpServer>(config);
  tcp_server->start();

  const uint32_t NUM_CLIENT = 2;

  config.strand->post([&]() {
    std::vector<shared_ptr_<Client>> clients;
    for (unsigned int i = 0; i < NUM_CLIENT; ++i) {
      shared_ptr_<Client> tcp_client(new Client(config));
      tcp_client->connect();
      clients.push_back(std::move(tcp_client));
    }

    boost::posix_time::milliseconds interval(20);
    boost::asio::deadline_timer timer(app.io_service(), interval);

    int waited = 0;

    std::function<void(const boost::system::error_code&)> wait_for_connected;
    wait_for_connected = [&](const boost::system::error_code& error) {
      ++waited;
      if (waited > 20) {  // waited for too long
        EXPECT_TRUE(false);
        app.force_stop();
      }
      for (auto& client : clients) {
        if (!client->get_session().is_connected()) {
          timer.async_wait(wait_for_connected);
          return;
        }
      }

      Server::destroy_in_strand(tcp_server);
      for (unsigned int i = 0; i < NUM_CLIENT; ++i) {
        Client::destroy_in_strand(clients[i]);
      }
      app.close();

    };

    timer.async_wait(wait_for_connected);

  });

  app.wait();
}

TEST(TcpServerTest, SingleStrand) {
  App app;

  WrapperConfig config = TestConfig(app).get_client_config(app);
  // use same config/strand for server and client
  config.tcp_server_port = config.tcp_port;

  auto tcp_server = make_shared_<TcpServer>(config);
  tcp_server->start();

  const uint32_t NUM_CLIENT = 2;

  std::vector<shared_ptr_<Client>> clients;
  for (unsigned int i = 0; i < NUM_CLIENT; ++i) {
    shared_ptr_<Client> tcp_client(new Client(config));
    tcp_client->connect();
    clients.push_back(std::move(tcp_client));
  }

  ASYNC_EXPECT_TRUE(500, *config.strand, [&]() {
    for (auto& client : clients) {
      if (!client->get_session().is_connected()) {
        return false;
      }
    }
    return true;
  });

  // close everything
  Server::destroy_in_strand(tcp_server);
  for (unsigned int i = 0; i < NUM_CLIENT; ++i) {
    Client::destroy_in_strand(clients[i]);
  }

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}

TEST(TcpServerTest, MultiStrand) {
  App app;

  TestConfig server_config(app);
  WrapperConfig client_config = server_config.get_client_config(app);

  //  auto tcp_server(new TcpServer(server_config));
  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  const uint32_t NUM_CLIENT = 2;

  std::vector<shared_ptr_<Client>> clients;
  for (unsigned int i = 0; i < NUM_CLIENT; ++i) {
    shared_ptr_<Client> tcp_client(new Client(client_config));
    tcp_client->connect();
    clients.push_back(std::move(tcp_client));
  }

  ASYNC_EXPECT_TRUE(500, *client_config.strand, [&]() {
    for (auto& client : clients) {
      if (!client->get_session().is_connected()) {
        return false;
      }
    }
    return true;
  });

  // close everything
  Server::destroy_in_strand(tcp_server);
  for (unsigned int i = 0; i < NUM_CLIENT; ++i) {
    Client::destroy_in_strand(clients[i]);
  }

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
