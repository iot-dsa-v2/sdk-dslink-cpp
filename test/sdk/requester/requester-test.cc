#include "dsa/message.h"
#include "dsa/network.h"
#include "gtest/gtest.h"

#include "network/client/tcp_client.h"
#include "network/server/tcp_server.h"

using namespace dsa;

TEST(RequesterTest, BasicFlow) {
  shared_ptr_<App> app;
  ECDH ecdh;

  ASSERT_NO_FATAL_FAILURE(app.reset(new App("RequesterTest")));

  app->async_start(2);

  Config server_config(&ecdh);
  server_config.tcp_host = "127.0.0.1";
  server_config.tcp_port = 8090;

  Config client_config = server_config;

  std::shared_ptr<Server> tcp_server(new TcpServer(app->new_strand(), server_config));
  tcp_server->start();

  app->sleep(500);

  shared_ptr_<TcpClient> tcp_client(
      new TcpClient(app->new_strand(), client_config));
  tcp_client->connect();

  // requester = client->session()->requester();

  // stream = requester.subscribe("/abc", func);
  // stream.close();

  // Construct a subscribe message request
  SubscribeRequestMessage subscribe_request;
  subscribe_request.set_qos(StreamQos::_2);
  subscribe_request.set_target_path("/path/name");

  shared_ptr_<Buffer> b = make_shared_<Buffer>(256);
  // subscribe_request.write(b->data());

  app->sleep(2000);

  tcp_server->close();
  tcp_client->close();
  app->close();
  app->wait();
}
