#include "dsa/network.h"
#include "dsa/message.h"
#include "gtest/gtest.h"

#include "network/tcp_server.h"
#include "network/tcp_connection.h"

using namespace dsa;

TEST(RequesterTest, basic_flow) {

  shared_ptr_<App> app;
  ASSERT_NO_FATAL_FAILURE(app.reset(new App("RequesterTest")));

  app->async_start(2);

  Server::Config server_config("/test/path", 8080);
  Client::Config client_config("127.0.0.1", 8080);


  std::shared_ptr<Server> tcp_server(new TcpServer(app->shared_from_this(), server_config));
  tcp_server->start();

  app->sleep(500);

  shared_ptr_<TcpClientConnection> tcp_client(new TcpClientConnection(app->shared_from_this(), client_config));
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

  tcp_server->stop();
  tcp_client->stop();
  app->stop();
  app->wait();
}
