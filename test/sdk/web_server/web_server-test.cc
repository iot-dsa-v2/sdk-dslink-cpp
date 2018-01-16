#include "dsa/stream.h"
#include "dsa_common.h"

#include <boost/beast/http.hpp>

#include "network/ws/ws_callback.h"
#include "network/ws/ws_client_connection.h"
#include "util/app.h"
#include "util/enable_shared.h"
#include "web_server/web_server.h"

#include "../async_test.h"
#include "../test_config.h"

#include <gtest/gtest.h>

using namespace dsa;

// TODO - webserver not to run separatedly
// TODO - stop all the work after calling app->close
TEST(WebServerTest, ws_subscribe) {
  auto app = make_shared_<App>();

  // client
  TestConfig test_config(app, false);
  WrapperStrand config;
  const string_ dsid_prefix = "ws_";
  config.ws_host = "127.0.0.1";
  config.ws_port = 8080;
  config.ws_path = "/";

  config.strand = EditableStrand::make_default(app);
  config.strand->logger().level = test_config.strand->logger().level;
  config.client_connection_maker = [
    dsid_prefix = dsid_prefix, ws_host = config.ws_host,
    ws_port = config.ws_port
  ](LinkStrandRef & strand) {
    return make_shared_<WsClientConnection>(strand, dsid_prefix, ws_host,
                                            ws_port);
  };

  ref_<Client> client(new Client(config));
  client->connect();

  ASYNC_EXPECT_TRUE(1000, *config.strand, [&]() {
    if (!client->get_session().is_connected()) {
      return false;
    }
    return true;
  });

  SubscribeOptions initial_options;
  initial_options.queue_duration = 0x1234;
  initial_options.queue_size = 0x5678;

  int msg_count = 0;
  ref_<const SubscribeResponseMessage> last_response;
  auto subscribe_stream = client->get_session().requester.subscribe(
      "downstream/test/value",
      [&](IncomingSubscribeStream &stream,
          ref_<const SubscribeResponseMessage> &&msg) {
        last_response = std::move(msg);  // store response
        ++msg_count;
      },
      initial_options);

  ASYNC_EXPECT_TRUE(1000, *config.strand,
                    [&]() -> bool { return last_response != nullptr; });

  app->close();

  // WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  // if (!app->is_stopped()) {
  //   app->force_stop();
  // }

  config.destroy();
  test_config.destroy();
  // app->wait();
}
