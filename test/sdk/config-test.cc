#include "dsa/network.h"

#include <atomic>

#include "core/client.h"
#include "network/tcp/tcp_server.h"

#include "async_test.h"
#include "test_config.h"

#include "gtest/gtest.h"

using namespace dsa;

TEST(ConfigTest, asyncSimpleSecurityManager) {
  App app;

  TestConfig server_strand(app);
  WrapperStrand client_strand = server_strand.get_client_wrapper_strand(app, true);

  auto tcp_server = make_shared_<TcpServer>(server_strand);
  tcp_server->start();

  const uint32_t NUM_CLIENT = 2;

  std::vector<ref_<Client>> clients;
  for (unsigned int i = 0; i < NUM_CLIENT; ++i) {
    ref_<Client> tcp_client(new Client(client_strand));
    tcp_client->connect();
    clients.push_back(std::move(tcp_client));
  }

  ASYNC_EXPECT_TRUE(500, *client_strand.strand, [&]() {
    for (auto& client : clients) {
      if (!client->get_session().is_connected()) {
        return false;
      }
    }
    return true;
  });

  // close everything
  tcp_server->destroy_in_strand(tcp_server);
  for (unsigned int i = 0; i < NUM_CLIENT; ++i) {
    destroy_client_in_strand(clients[i]);
  }

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
