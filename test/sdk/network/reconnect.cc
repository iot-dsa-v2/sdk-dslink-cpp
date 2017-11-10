#include "dsa/network.h"

#include <atomic>

#include "core/client.h"
#include "network/tcp/tcp_client_connection.h"
#include "network/tcp/tcp_server.h"

#include "../async_test.h"
#include "../test_config.h"

#include "gtest/gtest.h"

using namespace dsa;

TEST(NetworkTest, ReConnect) {
  App app;

  TestConfig server_config(app);
  WrapperConfig client_config = server_config.get_client_config(app);

  shared_ptr_<Connection> connection;
  client_config.client_connection_maker =
      [
        &connection, dsid_prefix = client_config.dsid_prefix,
        tcp_host = client_config.tcp_host, tcp_port = client_config.tcp_port
      ](LinkStrandRef & strand, const string_ &previous_session_id,
        int32_t last_ack_id)
          ->shared_ptr_<Connection> {
    connection =
        make_shared_<ClientConnection>(strand, dsid_prefix, tcp_host, tcp_port);
    return connection;
  };
  //  auto tcp_server(new TcpServer(server_config));
  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  ref_<Client> client(new Client(client_config));
  client->connect();

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return client->get_session().is_connected(); });

  connection->destroy_in_strand(connection);

  // it should be disconnected
  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return !client->get_session().is_connected(); });

  // it should get reconnected within 5 seconds
  ASYNC_EXPECT_TRUE(5000, *client_config.strand,
                    [&]() { return client->get_session().is_connected(); });

  // close everything
  tcp_server->destroy_in_strand(tcp_server);
  destroy_client_in_strand(client);

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
