#include "dsa/network.h"
#include "dsa/message.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(RequesterTest, basic_flow) {

  shared_ptr_<App> app;
  ASSERT_NO_FATAL_FAILURE(app.reset(new App("RequesterTest")));

  app->async_start(2);

  Server::Config server_config("/test/path", 8080);
  Client::Config client_config("127.0.0.1", 8080);


  ServerPtr tcp_server(app->new_server(Server::TCP, server_config));
  tcp_server->start();

  app->sleep(500);

  ClientPtr tcp_client(app->new_client(Client::TCP, client_config));
  tcp_client->connect();

  // requester = client->session()->requester();

  // stream = requester.subscribe("/abc", func);
  // stream.close();

  // Construct a subscribe message request
  SubscribeRequestMessage subscribe_request;
  subscribe_request.set_qos(StreamQos::_2);

  subscribe_request.set_target_path("/path/name");
  //?subscribe_request.update_static_header();


  app->sleep(2000);

  app->graceful_stop(1000);

  app->wait();
}
