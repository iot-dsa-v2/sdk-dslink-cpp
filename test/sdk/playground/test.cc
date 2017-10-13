#include "dsa/message.h"
#include "dsa/network.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"

using namespace dsa;

class MockNodeListChild : public NodeModelBase {
public:
  explicit MockNodeListChild(LinkStrandRef strand)
    : NodeModelBase(std::move(strand)){};
};

class MockNodeListRoot : public NodeModelBase {
public:
  std::unique_ptr<SubscribeOptions> first_subscribe_options;
  std::unique_ptr<SubscribeOptions> second_subscribe_options;

  explicit MockNodeListRoot(LinkStrandRef strand)
    : NodeModelBase(std::move(strand)){};

  void initialize() override {
    add_child("child_a", new MockNodeListChild(_strand));
    add_child("child_b", new MockNodeListChild(_strand));
  }
};

TEST(ResponderTest, ListTest) {
  App app;

  TestConfig server_config(app);

  MockNodeListRoot *root_node = new MockNodeListRoot(server_config.strand);

  server_config.get_link_config()->set_responder_model(
    ModelRef(root_node));

  WrapperConfig client_config = server_config.get_client_config(app);

  //  auto tcp_server(new TcpServer(server_config));
  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  auto tcp_client = make_shared_<Client>(client_config);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(50000000, *client_config.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  ref_<const ListResponseMessage> last_response;
  tcp_client->get_session().requester.list(
    "", [&](ref_<const ListResponseMessage> &&msg,
            IncomingListStream &stream) { last_response = msg; });

//  ASYNC_EXPECT_TRUE(500, *client_config.strand,
//                    [&]() { return last_response != nullptr; });

  Server::close_in_strand(tcp_server);
  Client::close_in_strand(tcp_client);

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
