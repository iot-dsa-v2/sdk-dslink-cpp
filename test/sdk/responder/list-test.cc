#include "dsa/message.h"
#include "dsa/network.h"
#include "dsa/responder.h"
#include "dsa/stream.h"

#include <module/logger.h>

#include <gtest/gtest.h>
#include "../async_test.h"
#include "../test_config.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"

using namespace dsa;

using ResponderTest = SetUpBase;

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
    add_list_child("Child_a", make_ref_<MockNodeChild>(_strand));
    add_list_child("Child_b", make_ref_<MockNodeChild>(_strand));
  };
};
}

TEST_F(ResponderTest, ListTest) {
  typedef responder_list_test::MockNodeRoot MockNodeRoot;
  typedef responder_list_test::MockNodeChild MockNodeChild;
  auto app = std::make_shared<App>();

  TestConfig server_strand(app, false, protocol());

  MockNodeRoot *root_node = new MockNodeRoot(server_strand.strand);

  server_strand.strand->set_responder_model(ModelRef(root_node));

  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  auto web_server = server_strand.create_webserver();
  web_server->start();

  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();

  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
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
      "Child_a",
      [&](IncomingListStream &stream, ref_<const ListResponseMessage> &&msg) {
        child_list_response = msg;
      });

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return root_list_response != nullptr; });
  {
    // check root list response
    auto mapref = root_list_response->get_parsed_map();
    auto map = *mapref;
    EXPECT_TRUE(map["Child_a"].is_map());
    EXPECT_EQ(map["Child_a"]["$is"].to_string(), "test_class");

    EXPECT_TRUE(map["Child_b"].is_map());
    EXPECT_EQ(map["Child_b"]["$is"].to_string(), "test_class");
    root_list_response.reset();
  }

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return child_list_response != nullptr; });
  {
    // check child list response
    auto mapref = child_list_response->get_parsed_map();
    auto map = *mapref;

    EXPECT_EQ(map["$is"].to_string(), "test_class");

    EXPECT_EQ(map["@unit"].to_string(), "test_unit");
  }

  // update root property
  server_strand.strand->post(
      [&]() { root_node->update_property("@int", Var(1)); });
  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
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
  server_strand.strand->post([&]() {
    root_node->add_list_child("Child_c",
                              new MockNodeChild(server_strand.strand));
  });
  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return root_list_response != nullptr; });
  {
    // check root list response
    auto mapref = root_list_response->get_parsed_map();
    auto map = *mapref;
    EXPECT_TRUE(map["Child_c"].is_map());
    EXPECT_EQ(map["Child_c"]["$is"].to_string(), "test_class");
  }

  // close list stream
  list_stream->close();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand, [&]() -> bool {
    return list_stream->is_destroyed() && list_stream->ref_count() == 1;
  });

  ASYNC_EXPECT_TRUE(1000, *server_strand.strand,
                    [&]() -> bool { return !root_node->need_list(); });

  tcp_server->destroy_in_strand(tcp_server);
  web_server->destroy();
  destroy_client_in_strand(tcp_client);

  client_strand.destroy();
  server_strand.destroy();
  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();
}
