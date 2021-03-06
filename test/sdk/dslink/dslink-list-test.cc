#include "dsa/message.h"
#include "dsa/network.h"
#include "dsa/responder.h"
#include "dsa/stream.h"

#include <gtest/gtest.h>
#include "../async_test.h"
#include "../test_config.h"

#include "../../broker/util/broker_runner.h"
#include "module/default/console_logger.h"
#include "network/tcp/tcp_server.h"

using namespace dsa;

using DslinkTest = SetUpBase;

namespace dslink_list_test {
class MockNodeChild : public NodeModel {
 public:
  explicit MockNodeChild(const LinkStrandRef &strand) : NodeModel(strand) {
    update_property("$is", Var("test_class"));
    update_property("@unit", Var("test_unit"));
  };
};

class MockNodeRoot : public NodeModel {
 public:
  bool need_list() { return _need_list; }

  explicit MockNodeRoot(const LinkStrandRef &strand) : NodeModel(strand) {
    add_list_child("child-a", make_ref_<MockNodeChild>(_strand));
    add_list_child("child-b", make_ref_<MockNodeChild>(_strand));
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

  bool flag1 = false;
  bool flag2 = false;
  bool flag3 = false;
  bool flag4 = false;
  bool is_connected = false;
  auto link = server_strand.create_dslink(true);
  ListResponses root_list_responses;
  ListResponses root_revisited_list_responses;
  link->connect([&](const shared_ptr_<Connection> connection,
                    ref_<DsLinkRequester> link_req) {
    is_connected = true;
    // list on root node
    auto list_cache1 = link_req->list("", [&, link_req = static_cast<ref_<DsLinkRequester>>(link_req->get_ref())](
                                              IncomingListCache & cache,
                                              const std::vector<string_> &str) {
      root_list_responses.push_back(str);
      if (!flag3) {
        EXPECT_TRUE(cache.get_map().size() != 0);
        {
          EXPECT_TRUE(root_list_responses.size() == 1);
          EXPECT_TRUE(root_list_responses[0].size() == 0);
          EXPECT_TRUE(cache.get_map().at("child-a").is_map());
          EXPECT_EQ(
              cache.get_map().at("child-a").get_map().at("$is").to_string(),
              "test_class");
          EXPECT_TRUE(cache.get_map().at("child-b").is_map());
          EXPECT_EQ(
              cache.get_map().at("child-b").get_map().at("$is").to_string(),
              "test_class");
        }
      } else {
        flag1 = true;
      }

      // list on child node
      auto list_cache2 = link_req->list(
          "child-a",
          [&](IncomingListCache &cache1, const std::vector<string_> &str) {
            if (!flag3) {
              EXPECT_TRUE(cache1.get_map().size() != 0);
              {
                EXPECT_EQ(cache1.get_map().at("$is").to_string(), "test_class");
                EXPECT_EQ(cache1.get_map().at("@unit").to_string(),
                          "test_unit");
              }
            } else {
              flag2 = true;
            }

            // update root child
            if (!flag3) root_list_responses.clear();
            if (!flag3) {
              server_strand.strand->post([&]() {
                root_node->add_list_child(
                    "child-c", new MockNodeChild(server_strand.strand));
                flag3 = true;
                //
              });
            }

          });

      if (flag1 && flag2 && flag3) {
        EXPECT_TRUE(root_list_responses.size() != 0);
        {
          EXPECT_TRUE(root_list_responses.size() == 1);
          EXPECT_TRUE(root_list_responses[0].size() == 1);
          EXPECT_TRUE(root_list_responses[0][0] == "child-c");
          EXPECT_TRUE(cache.get_map().at("child-c").is_map());
          EXPECT_EQ(
              cache.get_map().at("child-c").get_map().at("$is").to_string(),
              "test_class");
        }

        // list on root node revisited
        auto list_cache_revisited = link_req->list(
            "",
            [&](IncomingListCache &cache3, const std::vector<string_> &str) {
              root_revisited_list_responses.push_back(str);
              EXPECT_EQ(root_revisited_list_responses[0].size(), 0);
              EXPECT_TRUE(cache3.get_map() == cache.get_map());
              flag4 = true;
            });
      }

    });

  });
  ASYNC_EXPECT_TRUE(5000, *link->strand, [&]() {
    return (is_connected && flag1 && flag2 && flag3 && flag4);
  });

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
