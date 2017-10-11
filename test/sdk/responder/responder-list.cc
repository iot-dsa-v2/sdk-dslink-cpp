#include "dsa/message.h"
#include "dsa/network.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"

using namespace dsa;

class MockNodeList : public NodeModel {
 public:
  std::unique_ptr<SubscribeOptions> first_subscribe_options;
  std::unique_ptr<SubscribeOptions> second_subscribe_options;

  explicit MockNodeList(LinkStrandRef strand) : NodeModel(std::move(strand)){};

  void initialize() override {}
};

TEST(ResponderTest, List) {
  App app;

  TestConfig server_config(app);

  MockNodeList *root_node = new MockNodeList(server_config.strand);

  server_config.get_link_config()->set_responder_model(
      ref_<MockNodeList>(root_node));

  WrapperConfig client_config = server_config.get_client_config(app);

  //  auto tcp_server(new TcpServer(server_config));
  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  auto tcp_client = make_shared_<Client>(client_config);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  tcp_client->get_session().requester.list(
      "", [&](ref_<const ListResponseMessage> &&msg,
              IncomingListStream &stream) {

    });

  Server::close_in_strand(tcp_server);
  Client::close_in_strand(tcp_client);

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
