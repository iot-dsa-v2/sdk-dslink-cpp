#include "dsa/message.h"
#include "dsa/stream.h"
#include "dsa/network.h"
#include "dsa/responder.h"

#include <module/logger.h>

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

#include "core/client.h"
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

  /////////////////////
  // list on root node
  ListResponses root_list_responses;
  link->list("",
             [&](IncomingListCache &cache, const std::vector<string_> &str) {
               root_list_responses.push_back(str);
             });

  WAIT(500);
  EXPECT_EQ(root_list_responses.size(), 1);
  EXPECT_EQ(root_list_responses[0].size(), 2);
  EXPECT_CONTAIN(root_list_responses[0], "child_a");
  EXPECT_CONTAIN(root_list_responses[0], "child_b");

  /////////////////////
  // list on child node
  ListResponses child_list_responses;
  link->list("child_a",
             [&](IncomingListCache &cache, const std::vector<string_> &str) {
               child_list_responses.push_back(str);
             });

  WAIT(500);
  EXPECT_EQ(child_list_responses.size(), 1);
  EXPECT_EQ(child_list_responses[0].size(), 2);
  EXPECT_CONTAIN(child_list_responses[0], "$is");
  EXPECT_CONTAIN(child_list_responses[0], "@unit");

  /////////////////////
  // list on root node revisited
  ListResponses root_revisited_list_responses;
  link->list("",
             [&](IncomingListCache &cache, const std::vector<string_> &str) {
               root_revisited_list_responses.push_back(str);
             });

  WAIT(500);
  EXPECT_EQ(root_revisited_list_responses.size(), 1);
  EXPECT_EQ(root_revisited_list_responses[0].size(), 2);
  EXPECT_CONTAIN(root_revisited_list_responses[0], "child_a");
  EXPECT_CONTAIN(root_revisited_list_responses[0], "child_b");

  tcp_server->destroy_in_strand(tcp_server);
  destroy_dslink_in_strand(link);

  app->close();

  WAIT_EXPECT_TRUE(500, [&]() { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  server_strand.destroy();
  app->wait();
}
