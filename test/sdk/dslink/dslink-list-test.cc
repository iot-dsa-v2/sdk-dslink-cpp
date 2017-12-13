#include "dsa/message.h"
#include "dsa/stream.h"
#include "dsa/network.h"
#include "dsa/responder.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

#include "network/tcp/tcp_server.h"

using namespace dsa;

namespace dslink_list_test {
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
    add_list_child("child_a", make_ref_<MockNodeChild>(_strand));
    add_list_child("child_b", make_ref_<MockNodeChild>(_strand));
  };

//  void on_list(BaseOutgoingListStream &stream, bool first_request) override {
//    //std::cout<<"First Request : "<< first_request<<std::endl;
//  }
};

}

TEST(ResponderTest, ListTest) {
  typedef dslink_list_test::MockNodeRoot MockNodeRoot;
  typedef dslink_list_test::MockNodeChild MockNodeChild;
  typedef std::vector<std::vector<string_>> ListResponses;

  auto app = std::make_shared<App>();

  TestConfig server_strand(app);

  MockNodeRoot *root_node = new MockNodeRoot(server_strand.strand);

  server_strand.strand->set_responder_model(ModelRef(root_node));

  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  bool is_connected = false;
  auto link = server_strand.create_dslink(true);
  link->connect([&](const shared_ptr_<Connection> connection) { is_connected = true; });
  ASYNC_EXPECT_TRUE(500, *link->strand, [&]() { return is_connected; });

  // list on root node
  ListResponses root_list_responses;
  VarMap map;
  link->list("",
             [&](IncomingListCache &cache, const std::vector<string_> &str) {
               root_list_responses.push_back(str);
               map = cache.get_map();
             });

  ASYNC_EXPECT_TRUE(500, *link.get()->strand,
                    [&]() { return root_list_responses.size() == 1; });
  {
    EXPECT_CONTAIN(root_list_responses[0], "child_a");
    EXPECT_CONTAIN(root_list_responses[0], "child_b");
    EXPECT_TRUE(map["child_a"].is_map());
    EXPECT_EQ(map["child_a"]["$is"].to_string(), "test_class");
    EXPECT_TRUE(map["child_b"].is_map());
    EXPECT_EQ(map["child_b"]["$is"].to_string(), "test_class");
    EXPECT_EQ(root_list_responses[0].size(), 2);
  }

  // list on child node
  ListResponses child_list_responses;
  VarMap child_map;
  link->list("child_a",
             [&](IncomingListCache &cache, const std::vector<string_> &str) {
               child_list_responses.push_back(str);
               child_map = cache.get_map();
             });

  ASYNC_EXPECT_TRUE(500, *link.get()->strand,
                    [&]() { return child_list_responses.size() == 1; });
  {
    EXPECT_EQ(child_map["$is"].to_string(), "test_class");
    EXPECT_EQ(child_map["@unit"].to_string(), "test_unit");
  }
  // update root child
  server_strand.strand->post([&]() {
    root_node->add_list_child("child_c",
                              new MockNodeChild(server_strand.strand));
  });
  ASYNC_EXPECT_TRUE(500, *link.get()->strand,
                    [&]() { return root_list_responses.size() == 2; });
  {
    EXPECT_CONTAIN(root_list_responses[1], "child_c");
    EXPECT_TRUE(map["child_c"].is_map());
    EXPECT_EQ(map["child_c"]["$is"].to_string(), "test_class");
  }

  // list on root node revisited
  VarMap revisited_map;
  ListResponses root_revisited_list_responses;
  link->list("",
             [&](IncomingListCache &cache, const std::vector<string_> &str) {
               root_revisited_list_responses.push_back(str);
               revisited_map = cache.get_map();
             });

  ASYNC_EXPECT_TRUE(500, *link.get()->strand,
                    [&]() { return root_revisited_list_responses.size() == 1; });
  {
    EXPECT_EQ(root_revisited_list_responses[0].size(), 0);
    EXPECT_TRUE(revisited_map == map);
  }

  tcp_server->destroy_in_strand(tcp_server);
  destroy_dslink_in_strand(link);

  ASYNC_EXPECT_TRUE(500, *link.get()->strand, [&]() -> bool {
    return link->is_destroyed() && link->ref_count() == 1;
  });

  app->close();

  WAIT_EXPECT_TRUE(500, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  server_strand.destroy();
  app->wait();
}
