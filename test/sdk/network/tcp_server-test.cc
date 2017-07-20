#include "dsa/network.h"
#include "gtest/gtest.h"

#include <iostream>

using namespace dsa;

TEST(TcpServerTest, NewServer) {
  std::shared_ptr<App> app;
  ASSERT_NO_FATAL_FAILURE(app.reset(new App("Test")));

  Server::Config config;
  config.set_port(8080);

  app->add_server(Server::TCP, config);
  app->run();
}
