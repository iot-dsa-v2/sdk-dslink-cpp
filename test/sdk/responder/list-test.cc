#include "dsa/message.h"
#include "dsa/network.h"
#include "dsa/responder.h"

#include <module/logger.h>

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"

using namespace dsa;

namespace responder_list_test {
class MockNodeChild : public NodeModel {
 public:
  explicit MockNodeChild(LinkStrandRef strand) : NodeModel(std::move(strand)) {
    update_property("$is", Var("test_class"));
    update_property("@unit", Var("test_unit"));
  };
};

class MockNodeRoot : public NodeModel {
 public:
  bool need_list() { return _need_list; }

  explicit MockNodeRoot(LinkStrandRef strand) : NodeModel(std::move(strand)) {
    add_list_child("child_a", new MockNodeChild(_strand));
    add_list_child("child_b", new MockNodeChild(_strand));
  };
};
}

TEST(ResponderTest, ListTest) {
  typedef responder_list_test::MockNodeRoot MockNodeRoot;
  typedef responder_list_test::MockNodeChild MockNodeChild;
  App app;

  TestConfig server_config(app);

  MockNodeRoot *root_node = new MockNodeRoot(server_config.strand);

  server_config.strand->set_responder_model(ModelRef(root_node));

  WrapperConfig client_config = server_config.get_client_config(app);

  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  auto tcp_client = make_ref_<Client>(client_config);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  // list on root node
  ref_<const ListResponseMessage> root_list_response;
  auto list_stream = tcp_client->get_session().requester.list(
      "",
      [&](IncomingListStream &stream, ref_<const ListResponseMessage> &&msg) {
        root_list_response = msg;
      });

  // list on child node
  ref_<const ListResponseMessage> child_list_response;
  tcp_client->get_session().requester.list(
      "child_a",
      [&](IncomingListStream &stream, ref_<const ListResponseMessage> &&msg) {
        child_list_response = msg;
      });

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return root_list_response != nullptr; });
  {
    // check root list response
    auto mapref = root_list_response->get_parsed_map();
    auto map = *mapref;
    EXPECT_TRUE(map["child_a"].is_map());
    EXPECT_EQ(map["child_a"]["$is"].to_string(), "test_class");

    EXPECT_TRUE(map["child_b"].is_map());
    EXPECT_EQ(map["child_b"]["$is"].to_string(), "test_class");
    root_list_response.reset();
  }

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return child_list_response != nullptr; });
  {
    // check child list response
    auto mapref = child_list_response->get_parsed_map();
    auto map = *mapref;

    EXPECT_EQ(map["$is"].to_string(), "test_class");

    EXPECT_EQ(map["@unit"].to_string(), "test_unit");
  }

  // update root property
  server_config.strand->post(
      [&]() { root_node->update_property("@int", Var(1)); });
  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return root_list_response != nullptr; });
  {
    // check root list response
    auto mapref = root_list_response->get_parsed_map();
    auto map = *mapref;
    EXPECT_TRUE(map["@int"].is_int());
    EXPECT_EQ(map["@int"].get_int(), 1);
    root_list_response.reset();
  }

  // update root child
  server_config.strand->post([&]() {
    root_node->add_list_child("child_c",
                              new MockNodeChild(server_config.strand));
  });
  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return root_list_response != nullptr; });
  {
    // check root list response
    auto mapref = root_list_response->get_parsed_map();
    auto map = *mapref;
    EXPECT_TRUE(map["child_c"].is_map());
    EXPECT_EQ(map["child_c"]["$is"].to_string(), "test_class");
  }

  // close list stream
  list_stream->close();

  ASYNC_EXPECT_TRUE(500, *client_config.strand, [&]() -> bool {
    return list_stream->is_destroyed() && list_stream->ref_count() == 1;
  });

  ASYNC_EXPECT_TRUE(500, *server_config.strand,
                    [&]() -> bool { return !root_node->need_list(); });

  tcp_server->destroy_in_strand(tcp_server);
destroy_client_in_strand(tcp_client);

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
