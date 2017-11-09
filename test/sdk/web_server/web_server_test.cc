#include "gtest/gtest.h"

using namespace dsa;

TEST(WebServerTest, basic_flow) {

  App app;

  // auto web_server = make_shared_<WebServer>(app, cert);
  auto web_server = make_shared_<WebServer>(app);

  web_server->listen(port = 80);
  web_server->listen_secure(port=443);


  web_server->add_ws_handler(.... [](){
  new WsConnection();

});


}
