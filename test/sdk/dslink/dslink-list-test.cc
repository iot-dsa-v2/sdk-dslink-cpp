#include "dsa/message.h"
#include "dsa/stream.h"
#include "dsa/network.h"
#include "dsa/responder.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

#include "network/tcp/tcp_server.h"
#include "../../broker/util/broker_runner.h"

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

  WAIT_EXPECT_TRUE(500,
                    [&]() -> bool { return root_list_responses.size() == 1; });
  {
    EXPECT_TRUE(map["child_a"].is_map());
    EXPECT_EQ(map["child_a"]["$is"].to_string(), "test_class");
    EXPECT_TRUE(map["child_b"].is_map());
    EXPECT_EQ(map["child_b"]["$is"].to_string(), "test_class");
  }
  {
    EXPECT_CONTAIN(root_list_responses[0], "child_a");
    EXPECT_CONTAIN(root_list_responses[0], "child_b");
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

  WAIT_EXPECT_TRUE(500, [&]() { return child_list_responses.size() == 1; });
  {
    EXPECT_EQ(child_map["$is"].to_string(), "test_class");
    EXPECT_EQ(child_map["@unit"].to_string(), "test_unit");
  }
  // update root child
  server_strand.strand->post([&]() {
    root_node->add_list_child("child_c",
                              new MockNodeChild(server_strand.strand));
  });
  WAIT_EXPECT_TRUE(500, [&]() { return root_list_responses.size() == 2; });
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

  WAIT_EXPECT_TRUE(500, [&]() { return root_revisited_list_responses.size() == 1; });
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




TEST(DSLinkTest, DisconnectTest) {
  // First Create Broker
  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();

  std::string address = std::string("127.0.0.1:") + std::to_string(broker->tcp_server_port);
  const char *argv[] = {"./test", "-b", address.c_str()};
  int argc = 3;
  auto link_1 = make_ref_<DsLink>(argc, argv, "test1", "1.0.0", app);
  link_1->init_responder();

  auto link_2 = make_ref_<DsLink>(argc, argv, "test2", "1.0.0", app);
  link_2->init_responder();

  // after client1 disconnected, list update should show it's disconnected
  auto step_3_disconnection_list = [&]() {
    link_2->list(
        "downstream/test1",
        [&](IncomingListCache &cache, const std::vector<string_> &str) {
          EXPECT_EQ(cache.get_status(), MessageStatus::NOT_AVAILABLE);
          // end the test

          link_2->strand->post([link_2]() { link_2->destroy(); });
          broker->strand->post([broker]() { broker->destroy(); });
        });
  };

  // downstream should has test1 and test2 nodes
  auto step_2_downstream_list = [&]() {
    link_2->list(
        "downstream",
        [&](IncomingListCache &cache, const std::vector<string_> &str) {
          auto map = cache.get_map();
          EXPECT_TRUE(map["test1"].is_map());
          EXPECT_TRUE(map["test2"].is_map());
          step_3_disconnection_list();
        });
  };

  // when list on downstream/test1 it should have a metadata for test1's dsid
  auto step_1_downstream_child_list = [&]() {
        link_1->list(
            "downstream/test1",
            [&](IncomingListCache &cache, const std::vector<string_> &str) {
              auto map = cache.get_map();
              //std::cout<<"dsid : "<< map["$$dsid"].get_string()<<std::endl;
//              EXPECT_EQ(map["$$dsid"].to_string(), link_1->dsid());

              link_1->strand->post([link_1]() {link_1->destroy();});
              step_2_downstream_list();
            });

      };

  std::mutex mutex;
  bool one_of_them_connected = false;
  link_1->connect([&](const shared_ptr_<Connection> connection) {
    //std::cout<<"Hello1"<<std::endl;
    std::lock_guard<std::mutex> lock{mutex};
    if(one_of_them_connected) step_1_downstream_child_list();
    one_of_them_connected = true;
  });
  link_2->connect([&](const shared_ptr_<Connection> connection) {
    //std::cout<<"Hello2"<<std::endl;
    std::lock_guard<std::mutex> lock{mutex};
    if(one_of_them_connected) step_1_downstream_child_list();
    one_of_them_connected = true;
  });

  broker->run();
  EXPECT_TRUE(broker->is_destroyed());
}
