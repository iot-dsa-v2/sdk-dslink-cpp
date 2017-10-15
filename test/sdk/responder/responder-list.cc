#include <module/logger.h>
#include "dsa/message.h"
#include "dsa/network.h"
#include "dsa/responder.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"

using namespace dsa;

class MockNodeListChild : public NodeModel {
 public:
  explicit MockNodeListChild(LinkStrandRef strand)
      : NodeModel(std::move(strand)) {
    update_property("$is", Variant("test_class"));
    update_property("@unit", Variant("test_unit"));
  };
};

class MockNodeListRoot : public NodeModel {
 public:
  std::unique_ptr<SubscribeOptions> first_subscribe_options = nullptr;
  std::unique_ptr<SubscribeOptions> second_subscribe_options = nullptr;

  explicit MockNodeListRoot(LinkStrandRef strand)
      : NodeModel(std::move(strand)) {
    add_list_child("child_a", new MockNodeListChild(_strand));
    add_list_child("child_b", new MockNodeListChild(_strand));
  };
};

TEST(ResponderTest, ListTest) {
  App app;

  TestConfig server_config(app);

  MockNodeListRoot *root_node = new MockNodeListRoot(server_config.strand);

  server_config.get_link_config()->set_responder_model(ModelRef(root_node));

  WrapperConfig client_config = server_config.get_client_config(app);

  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  auto tcp_client = make_shared_<Client>(client_config);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  // list on root node
  ref_<const ListResponseMessage> root_list_response;
  tcp_client->get_session().requester.list(
      "", [&](ref_<const ListResponseMessage> &&msg,
              IncomingListStream &stream) { root_list_response = msg; });

  // list on child node
  ref_<const ListResponseMessage> child_list_response;
  tcp_client->get_session().requester.list(
      "child_a",
      [&](ref_<const ListResponseMessage> &&msg, IncomingListStream &stream) {
        child_list_response = msg;
      });

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return root_list_response != nullptr; });
  {
    // check root list response
    auto mapref = root_list_response->get_map();
    auto map = *mapref;
    EXPECT_TRUE(map["child_a"].is_map());
    EXPECT_TRUE(map["child_a"]["$is"].is_string());
    EXPECT_EQ(map["child_a"]["$is"].get_string(), "test_class");

    EXPECT_TRUE(map["child_b"].is_map());
    EXPECT_TRUE(map["child_b"]["$is"].is_string());
    EXPECT_EQ(map["child_b"]["$is"].get_string(), "test_class");
    root_list_response.reset();
  }

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return child_list_response != nullptr; });
  {
    // check child list response
    auto mapref = child_list_response->get_map();
    auto map = *mapref;

    EXPECT_TRUE(map["$is"].is_string());
    EXPECT_EQ(map["$is"].get_string(), "test_class");

    EXPECT_TRUE(map["@unit"].is_string());
    EXPECT_EQ(map["@unit"].get_string(), "test_unit");
  }

  // update root property
  server_config.strand->post(
      [&]() { root_node->update_property("@int", Variant(1)); });
  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return root_list_response != nullptr; });
  {
    // check root list response
    auto mapref = root_list_response->get_map();
    auto map = *mapref;
    EXPECT_TRUE(map["@int"].is_int());
    EXPECT_EQ(map["@int"].get_int(), 1);
    root_list_response.reset();
  }

  // update root child
  server_config.strand->post([&]() {
    root_node->add_list_child("child_c",
                              new MockNodeListChild(server_config.strand));
  });
  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return root_list_response != nullptr; });
  {
    // check root list response
    auto mapref = root_list_response->get_map();
    auto map = *mapref;
    EXPECT_TRUE(map["child_c"].is_map());
    EXPECT_TRUE(map["child_c"]["$is"].is_string());
    EXPECT_EQ(map["child_c"]["$is"].get_string(), "test_class");
  }

  Server::close_in_strand(tcp_server);
  Client::close_in_strand(tcp_client);

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
