#include "dsa/crypto.h"
#include "dsa/responder.h"

#include <gtest/gtest.h>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <thread>
#include "../test_config.h"
#include "dslink.h"
#include "module/default/console_logger.h"
#include "module/logger.h"
#include "util/string.h"

#include "../async_test.h"
#include "dsa/stream.h"

#include "message/request/invoke_request_message.h"

using boost::format;

using namespace dsa;
using namespace std;

using DslinkTest = SetUpBase;
class MockNodeChild : public NodeModel {
 public:
  explicit MockNodeChild(const LinkStrandRef &strand, Var v)
      : NodeModel(strand) {
    update_property("$is", Var(v));
  };
  bool save_child(const string_ &name) const override { return true; };
};

class MockNodeMain : public NodeModel {
 public:
  explicit MockNodeMain(const LinkStrandRef &strand) : NodeModel(strand){};

  bool save_child(const string_ &name) const override {
    if (name == "add-child")
      return false;
    else
      return true;
  };

  void on_load_child(const string_ &name, VarMap &map) override {
    add_list_child(
        name, make_ref_<MockNodeChild>(this->_strand, Var(name)));
  };
};

TEST_F(DslinkTest, SaveMainNode) {
  shared_ptr<App> app = make_shared<App>();

  const char *argv[] = {"./testResp", "--broker",      "ds://127.0.0.1:4121",
                        "-l",         "info",          "--thread",
                        "4",          "--server-port", "4121"};
  int argc = 9;
  auto link = make_ref_<DsLink>(argc, argv, "mydslink", "1.0.0", app);
  // filter log for unit test
  static_cast<ConsoleLogger &>(Logger::_()).filter =
      Logger::FATAL_ | Logger::ERROR_ | Logger::WARN__;

  ref_<MockNodeMain> main_node =
      make_ref_<MockNodeMain>(link->strand);
  main_node->add_list_child(
      "add-child",
      make_ref_<SimpleInvokeNode>(
          link->strand,
          [&](Var &&v, SimpleInvokeNode &node, OutgoingInvokeStream &stream,
              ref_<NodeState> &&parent) {

            auto *parent_model = parent->model_cast<NodeModel>();
            // add new child node
            parent_model->add_list_child(
                v.to_string(),
                make_ref_<MockNodeChild>(link->strand, v));

            stream.close();
          }));

  link->init_responder(main_node);
  bool invoked = false;
  bool invoked2 = false;
  ref_<IncomingListCache> list_cache;
  link->connect([&](const shared_ptr_<Connection> connection,
                    ref_<DsLinkRequester> link_req) {

    // invoke the Add_Child node to add new child
    auto request = make_ref_<InvokeRequestMessage>();
    request->set_target_path("main/add-child");
    request->set_body(Var("child-a").to_msgpack());
    link_req->invoke(
        [&](IncomingInvokeStream &, ref_<const InvokeResponseMessage> &&msg) {
          EXPECT_EQ(msg->get_status(), Status::DONE);
          invoked = true;
        },
        std::move(request));

    // invoke the Add_Child node to add second child
    auto request2 = make_ref_<InvokeRequestMessage>();
    request2->set_target_path("main/add-child");
    request2->set_body(Var("child-b").to_msgpack());
    link_req->invoke(
        [&](IncomingInvokeStream &, ref_<const InvokeResponseMessage> &&msg) {
          EXPECT_EQ(msg->get_status(), Status::DONE);
          invoked2 = true;
        },
        std::move(request2));

  });

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return invoked && invoked2; });

  SimpleStorage simple_storage(nullptr);

  std::string storage_key("vk123");

  shared_ptr_<StorageBucket> storage_bucket =
      simple_storage.get_shared_bucket("node");

  storage_bucket->remove_all();
  //save main node
  main_node->save(*storage_bucket, "main", false, true);

  destroy_dslink_in_strand(link);
  main_node.reset();

  typedef std::vector<std::vector<string_>> ListResponses;
  auto link2 = make_ref_<DsLink>(argc, argv, "mydslink", "1.0.0", app);
  // filter log for unit test
  static_cast<ConsoleLogger &>(Logger::_()).filter =
      Logger::FATAL_ | Logger::ERROR_ | Logger::WARN__;

  ref_<MockNodeMain> main_node2 =
      make_ref_<MockNodeMain>(link2->strand);
  main_node2->add_list_child(
      "add-child",
      make_ref_<SimpleInvokeNode>(
          link2->strand,
          [&](Var &&v, SimpleInvokeNode &node, OutgoingInvokeStream &stream,
              ref_<NodeState> &&parent) {

            auto *parent_model = parent->model_cast<NodeModel>();

            parent_model->add_list_child(
                v.to_string(),
                make_ref_<MockNodeChild>(link2->strand, v));

            stream.close();
          }));

  bool connected = false;
  bool flag1 = false;
  ListResponses root_list_responses;
  ref_<IncomingListCache> list_cache2;

  SimpleStorage simple_storage2(nullptr);

  shared_ptr_<StorageBucket> storage_bucket2 =
      simple_storage2.get_shared_bucket("node");

  int read_order = 0;
  auto read_all_callback = [&](std::string storage_key,
                               std::vector<uint8_t> vec,
                               BucketReadStatus read_status) {

    if (storage_key == "main") {
      std::string str(vec.begin(), vec.end());

      auto v = Var::from_json(str);
      //load main node
      main_node2->load(v.get_map());
    }

    return;
  };

  storage_bucket2->read_all(read_all_callback, [&]() {
    EXPECT_EQ(read_order, 0);
    return;
  });

  link2->init_responder(std::move(main_node2));
  link2->connect([&](const shared_ptr_<Connection> connection,
                    ref_<DsLinkRequester> link_req) {
    connected = true;

    // list on Main node
    auto list_cache1 = link_req->list(
        "main",
        [
              &,
              link_req = static_cast<ref_<DsLinkRequester>>(link_req->get_ref())
        ](IncomingListCache & cache, const std::vector<string_> &str) {
          root_list_responses.push_back(str);
          //verify structure of Main node
          EXPECT_TRUE(cache.get_map().size() != 0);
          {
            EXPECT_TRUE(root_list_responses.size() == 1);
            EXPECT_TRUE(root_list_responses[0].size() == 0);
            EXPECT_TRUE(cache.get_map().at("child-a").is_map());
            EXPECT_EQ(
                cache.get_map().at("child-a").get_map().at("$is").to_string(),
                "child-a");
            EXPECT_TRUE(cache.get_map().at("child-b").is_map());
            EXPECT_EQ(
                cache.get_map().at("child-b").get_map().at("$is").to_string(),
                "child-b");
          }
          flag1 = true;
        });
  });

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return connected && flag1; });

  destroy_dslink_in_strand(link2);
  storage_bucket2->remove_all();

  app->close();
  WAIT_EXPECT_TRUE(3000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();
}
