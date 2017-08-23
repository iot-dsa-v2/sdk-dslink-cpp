#include "dsa/message.h"
#include "dsa/network.h"
#include "gtest/gtest.h"

#include "network/tcp/tcp_client.h"
#include "network/tcp/tcp_server.h"

#include "module/default_modules.h"

using namespace dsa;

TEST(RequesterTest, BasicFlow) {
  WrapperConfig config;
  config.tcp_host = "127.0.0.1";
  config.tcp_port = 8090;

  App app("BasicFlowTest");
  app.async_start(2);

  auto tcp_server = make_shared_<TcpServer>(config);
  tcp_server->start();

  app.sleep(500);

  auto tcp_client = make_shared_<TcpClient>(config);
  tcp_client->connect();

  // requester = client->session()->requester();

  // stream = requester.subscribe("/abc", func);
  // stream.close();

  // Construct a subscribe message request
  SubscribeRequestMessage subscribe_request;
  subscribe_request.set_qos(StreamQos::_2);
  subscribe_request.set_target_path("/path/name");

  shared_ptr_<ByteBuffer> b = make_shared_<ByteBuffer>(256);
  // subscribe_request.write(b->data());

  app.sleep(2000);

  tcp_server->close();
  tcp_client->close();
  app.close();
  app.wait();
}
