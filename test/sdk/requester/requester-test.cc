#include "dsa/network.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(RequesterTest, basic_flow) {

  std::shared_ptr<App> appPtr;
  ASSERT_NO_FATAL_FAILURE(appPtr.reset(new App("RequesterTest")));

  appPtr->async_start(2);

  Server::Config server_config("/test/path", 8080);
  Client::Config client_config("127.0.0.1", 8080);


  ServerPtr tcp_server = appPtr->new_server(Server::TCP, server_config);
  tcp_server->start();

  appPtr->sleep(1000);

  ClientPtr tcp_client = appPtr->new_client(Client::TCP, client_config);
  tcp_client->connect();

  appPtr->sleep(2000);

  appPtr->wait();
}
