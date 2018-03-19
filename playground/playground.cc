#include "dsa/network.h"
#include "dsa/responder.h"
#include "dsa/stream.h"

#include <gtest/gtest.h>
#include "../test/sdk/async_test.h"
#include "../test/sdk/test_config.h"
#include "network/tcp/tcp_server.h"

#include "module/default/simple_security.h"
#include "module/default/simple_session_manager.h"
#include "module/default/console_logger.h"

using namespace dsa;

TEST(NetworkTest, ReConnect) {
  auto app = std::make_shared<App>();

  TestConfig server_strand(app);

  server_strand.strand->set_responder_model(
      make_ref_<NodeModel>(server_strand.strand));

  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();

  auto client = make_ref_<Client>(client_strand);
  client->connect();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return client->get_session().is_connected(); });

  // close everything
  tcp_server->destroy_in_strand(tcp_server);
  destroy_client_in_strand(client);

  server_strand.destroy();
  client_strand.destroy();

  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();
}
