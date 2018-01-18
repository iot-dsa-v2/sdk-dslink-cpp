#include "dsa/message.h"
#include "dsa/network.h"
#include "dsa/stream.h"

#include <atomic>

#include "../async_test.h"
#include "../test_config.h"
#include "core/client.h"
#include "module/logger.h"
#include "network/tcp/tcp_client_connection.h"
#include "network/tcp/tcp_server.h"
#include "responder/node_model.h"

#include "gtest/gtest.h"

using namespace dsa;

namespace network_reconnect_test {

/// define a node for the responder
class MockNode : public NodeModel {
 public:
  explicit MockNode(
      LinkStrandRef strand)  // allows set value with write permission
      : NodeModel(std::move(strand), PermissionLevel::WRITE) {
    update_property("$type", Var("string"));
    set_value(Var("hello"));
  };
};
}
TEST(NetworkTest, ReConnect) {
  typedef network_reconnect_test::MockNode MockNode;

  auto app = std::make_shared<App>();

  TestConfig server_strand(app);

  MockNode *root_node = new MockNode(server_strand.strand);

  server_strand.strand->set_responder_model(ModelRef(root_node));

  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();

  shared_ptr_<Connection> connection;
  client_strand.client_connection_maker =
      [
        &connection, dsid_prefix = client_strand.dsid_prefix,
        tcp_host = client_strand.tcp_host, tcp_port = client_strand.tcp_port
      ](LinkStrandRef & strand)
          ->shared_ptr_<Connection> {
    connection = make_shared_<TcpClientConnection>(strand, dsid_prefix,
                                                   tcp_host, tcp_port);
    return connection;
  };

  auto client = make_ref_<Client>(client_strand);
  client->connect();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return client->get_session().is_connected(); });

  // subscribe on root node value
  ref_<const SubscribeResponseMessage> last_subscribe_response;
  auto subscribe_stream = client->get_session().requester.subscribe(
      "", [&](IncomingSubscribeStream &stream,
              ref_<const SubscribeResponseMessage> &&msg) {
        last_subscribe_response = std::move(msg);  // store response
      });

  // list on root node
  ref_<const ListResponseMessage> last_list_response;
  client->get_session().requester.list(
      "",
      [&](IncomingListStream &stream, ref_<const ListResponseMessage> &&msg) {
        last_list_response = msg;
      });

  // waiting until list stream and subscribe stream are working
  ASYNC_EXPECT_TRUE(1000, *client_strand.strand, [&]() {
    return last_list_response != nullptr && last_subscribe_response != nullptr;
  });

  connection->destroy_in_strand(connection);

  // clear cached response
  client_strand.strand->dispatch([&]() {
    last_list_response.reset();
    last_subscribe_response.reset();
  });

  // it should be disconnected
  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return !client->get_session().is_connected(); });

  // update list stream and subscribe stream
  server_strand.strand->dispatch([&]() {
    root_node->set_value(Var("world"));
    root_node->update_property("@test", Var("test"));
  });

  // it should get reconnected within 5 seconds
  ASYNC_EXPECT_TRUE(10000, *client_strand.strand,
                    [&]() { return client->get_session().is_connected(); });

  // subscribe stream and list stream should get new value after reconnect
  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return last_list_response != nullptr; });

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return last_subscribe_response != nullptr; });

  // close everything
  tcp_server->destroy_in_strand(tcp_server);
  destroy_client_in_strand(client);

  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  server_strand.destroy();
  client_strand.destroy();
  app->wait();
}
