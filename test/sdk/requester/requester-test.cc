#include "dsa/message.h"
#include "dsa/network.h"
#include "gtest/gtest.h"

#include "network/tcp/tcp_client.h"
#include "network/tcp/tcp_server.h"

#include "module/default_modules.h"

using namespace dsa;

TEST(RequesterTest, Subscribe) {
  App app;

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

  auto tcp_client = make_shared_<TcpClient>(client_config);
  tcp_client->connect();

  std::atomic_bool connected{false};

  // wait till all client is connected
  while (!connected) {
    (*client_config.strand)()->dispatch(
        [&]() { connected = tcp_client->get_session().is_connected(); });
    app.sleep(50);
  }

  tcp_client->get_session().requester.subscribe(
      "/path", [](ref_<SubscribeResponseMessage>&& msg) {});


  app.sleep(500);

  Server::close_in_strand(tcp_server);
  Client::close_in_strand(tcp_client);

  app.close();
  for (int i = 0; i < 10; ++i) {
    app.sleep(50);
    if (app.is_stopped()) {
      break;
    }
  }

  EXPECT_TRUE(app.is_stopped());

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
