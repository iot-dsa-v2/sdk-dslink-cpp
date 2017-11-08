
#include "gtest/gtest.h"

using namespace dsa;

TEST(BufferTest, output) {

auto web_server = make_shared_<WebServer>(cert);
web_server->listen(port = 80);
web_server->listen_secure(port=443);


web_server->add_ws_handler(.... [](){
  new WsConnection();
});



}