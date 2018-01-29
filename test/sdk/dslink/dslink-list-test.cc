#include "dsa/message.h"
#include "dsa/network.h"
#include "dsa/responder.h"
#include "dsa/stream.h"

#include "../async_test.h"
#include "../test_config.h"
#include <gtest/gtest.h>

#include "../../broker/util/broker_runner.h"
#include "module/default/console_logger.h"
#include "network/tcp/tcp_server.h"

using namespace dsa;

using DslinkTest = SetUpBase;

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

  //  void on_list(BaseOutgoingListStream &stream, bool first_request) override
  //  {
  //    //std::cout<<"First Request : "<< first_request<<std::endl;
  //  }
};
}

TEST_F(DslinkTest, ListTest) {
  typedef dslink_list_test::MockNodeRoot MockNodeRoot;
  typedef dslink_list_test::MockNodeChild MockNodeChild;
  typedef std::vector<std::vector<string_>> ListResponses;

  auto app = std::make_shared<App>();

  TestConfig server_strand(app, false, protocol());

  MockNodeRoot *root_node = new MockNodeRoot(server_strand.strand);

  server_strand.strand->set_responder_model(ModelRef(root_node));

  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  auto web_server = server_strand.create_webserver();
  web_server->start();

  bool is_connected = false;
  auto link = server_strand.create_dslink(true);
  link->connect(
      [&](const shared_ptr_<Connection> connection) { is_connected = true; });
  ASYNC_EXPECT_TRUE(1000, *link->strand, [&]() { return is_connected; });

  // list on root node
  ListResponses root_list_responses;
  auto list_cache1 = link->list("",
             [&](IncomingListCache &cache, const std::vector<string_> &str) {
               root_list_responses.push_back(str);
             });

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return list_cache1->get_map().size() != 0; });
  {
    EXPECT_TRUE(root_list_responses.size() == 1);
    EXPECT_TRUE(root_list_responses[0].size() == 0);
    EXPECT_TRUE(list_cache1->get_map().at("child_a").is_map());
    EXPECT_EQ(list_cache1->get_map().at("child_a").get_map().at("$is").to_string(), "test_class");
    EXPECT_TRUE(list_cache1->get_map().at("child_b").is_map());
    EXPECT_EQ(list_cache1->get_map().at("child_b").get_map().at("$is").to_string(), "test_class");
  }

  // list on child node
  auto list_cache2 = link->list("child_a",
             [&](IncomingListCache &cache, const std::vector<string_> &str) {
             });

  WAIT_EXPECT_TRUE(1000, [&]() { return list_cache2->get_map().size() != 0; });
  {
    EXPECT_EQ(list_cache2->get_map().at("$is").to_string(), "test_class");
    EXPECT_EQ(list_cache2->get_map().at("@unit").to_string(), "test_unit");
  }

  // update root child
  root_list_responses.clear();

  server_strand.strand->post([&]() {
    root_node->add_list_child("child_c",
                              new MockNodeChild(server_strand.strand));
  });
  WAIT_EXPECT_TRUE(1000, [&]() { return root_list_responses.size() != 0; });
  {
    EXPECT_TRUE(root_list_responses.size() == 1);
    EXPECT_TRUE(root_list_responses[0].size() == 1);
    EXPECT_TRUE(root_list_responses[0][0] == "child_c");
    EXPECT_TRUE(list_cache1->get_map().at("child_c").is_map());
    EXPECT_EQ(list_cache1->get_map().at("child_c").get_map().at("$is").to_string(), "test_class");
  }

  // list on root node revisited
  ListResponses root_revisited_list_responses;
  auto list_cache_revisited = link->list("",
             [&](IncomingListCache &cache, const std::vector<string_> &str) {
               root_revisited_list_responses.push_back(str);
             });

  WAIT_EXPECT_TRUE(1000,
                   [&]() { return root_revisited_list_responses.size() == 1; });
  {
    EXPECT_EQ(root_revisited_list_responses[0].size(), 0);
    EXPECT_TRUE(list_cache_revisited->get_map() == list_cache1->get_map());
  }

  tcp_server->destroy_in_strand(tcp_server);
  web_server->destroy();
  destroy_dslink_in_strand(link);

  ASYNC_EXPECT_TRUE(1000, *link.get()->strand, [&]() -> bool {
    return link->is_destroyed() && link->ref_count() == 1;
  });

  app->close();

  server_strand.destroy();
  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();
}
