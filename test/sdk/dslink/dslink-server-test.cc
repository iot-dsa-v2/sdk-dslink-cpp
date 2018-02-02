#include "dsa/network.h"
#include "dsa/responder.h"
#include "dsa/stream.h"
#include "dslink.h"

#include <util/string.h>
#include "../async_test.h"
#include "../test_config.h"

#include <gtest/gtest.h>
#include "message/request/invoke_request_message.h"
#include "module/default/console_logger.h"

using namespace dsa;
using namespace std;

using DslinkTest = SetUpBase;

class ExampleNodeChild : public NodeModel {
 public:
  explicit ExampleNodeChild(LinkStrandRef strand)
      : NodeModel(std::move(strand)) {
    update_property("$type", Var("string"));
    update_property("@attr", Var("test attribute value"));
    set_value(Var("test string value 1"));
  };
};

class ExampleNodeRoot : public NodeModel {
 public:
  explicit ExampleNodeRoot(LinkStrandRef strand)
      : NodeModel(std::move(strand)) {
    add_list_child("child_a", new ExampleNodeChild(_strand));
    add_list_child("child_b", new ExampleNodeChild(_strand));
  };
};

TEST_F(DslinkTest, ServerTest) {
  shared_ptr<App> app = make_shared<App>();

  const char *argv[] = {"./testResp", "--broker",      "ds://127.0.0.1:4120",
                        "-l",         "info",          "--thread",
                        "4",          "--server-port", "4121"};
  int argc = 9;
  auto linkResp = make_ref_<DsLink>(argc, argv, "mydslink", "1.0.0");
  // filter log for unit test
  static_cast<ConsoleLogger &>(linkResp->strand->logger()).filter =
      Logger::FATAL_ | Logger::ERROR_ | Logger::WARN__;

  linkResp->init_responder<ExampleNodeRoot>();
  linkResp->connect();

  // Create link
  std::string address = std::string("127.0.0.1:") + std::to_string(4121);

  const char *argv2[] = {"./test", "-b", address.c_str()};
  int argc2 = 3;
  auto link = make_ref_<DsLink>(argc2, argv2, "mydslink", "1.0.0", app);

  // filter log for unit test
  static_cast<ConsoleLogger &>(link->strand->logger()).filter =
      Logger::FATAL_ | Logger::ERROR_ | Logger::WARN__;

  // connection
  bool is_connected = false;
  link->connect(
      [&](const shared_ptr_<Connection> connection) { is_connected = true; });

  ASYNC_EXPECT_TRUE(1000, *link->strand, [&]() { return is_connected; });

  std::vector<string_> list_result;
  // List test
  auto list_cache1 = link->list(
      "", [&](IncomingListCache &cache, const std::vector<string_> &str) {});
  WAIT_EXPECT_TRUE(1000, [&]() { return list_cache1->get_map().size() != 0; });
  EXPECT_NE(list_cache1->get_map().find("sys"), list_cache1->get_map().end());
  EXPECT_NE(list_cache1->get_map().find("pub"), list_cache1->get_map().end());
  EXPECT_NE(list_cache1->get_map().find("main"), list_cache1->get_map().end());

  // add a callback when connected to broker
  std::vector<std::string> messages;
  link->subscribe("main/child_a",
                  [&](IncomingSubscribeCache &cache,
                      ref_<const SubscribeResponseMessage> message) {
                    messages.push_back(message->get_value().value.get_string());
                  });
  WAIT_EXPECT_TRUE(1000, [&]() { return messages.size() == 1; });
  EXPECT_TRUE(messages.size() == 1);
  EXPECT_EQ(messages.at(0), "test string value 1");

  // Cleaning test
  destroy_dslink_in_strand(linkResp);
  destroy_dslink_in_strand(link);

  app->close();
  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();
}

TEST_F(DslinkTest, CloseTest) {
  shared_ptr<App> app = make_shared<App>();

  // first create .close_token
  string_ close_token = "12345678901234567890123456789012";
  try {
    close_token = string_from_file(".close_token");
  } catch (std::exception &e) {
    string_to_file(close_token, ".close_token");
  }

  const char *argv[] = {"./testResp", "--broker",      "ds://127.0.0.1:4122",
                        "-l",         "info",          "--thread",
                        "4",          "--server-port", "4122"};
  int argc = 9;
  auto linkResp = make_ref_<DsLink>(argc, argv, "mydslink", "1.0.0");
  // filter log for unit test
  static_cast<ConsoleLogger &>(linkResp->strand->logger()).filter =
      Logger::FATAL_ | Logger::ERROR_ | Logger::WARN__;

  linkResp->init_responder<ExampleNodeRoot>();
  linkResp->connect([&](const shared_ptr_<Connection> connection) {});

  EXPECT_EQ(close_token, linkResp->get_close_token());

  // Create link
  std::string address = std::string("127.0.0.1:") + std::to_string(4122);

  const char *argv2[] = {"./test", "-b", address.c_str()};
  int argc2 = 3;
  auto link = make_ref_<DsLink>(argc2, argv2, "mydslink", "1.0.0", app);
  // filter log for unit test
  static_cast<ConsoleLogger &>(link->strand->logger()).filter =
      Logger::FATAL_ | Logger::ERROR_ | Logger::WARN__;

  // connection
  bool is_connected = false;
  link->connect(
      [&](const shared_ptr_<Connection> connection) { is_connected = true; });

  ASYNC_EXPECT_TRUE(1000, *link->strand, [&]() { return is_connected; });

  auto close_request_with_wrong_token = make_ref_<InvokeRequestMessage>();
  close_request_with_wrong_token->set_value(Var("wrongtoken"));
  close_request_with_wrong_token->set_target_path("sys/stop");

  ref_<const InvokeResponseMessage> response_invoke_failed;
  link->invoke(
      [&](IncomingInvokeStream &stream,
          ref_<const InvokeResponseMessage> &&msg) {
        response_invoke_failed = std::move(msg);
      },
      copy_ref_(close_request_with_wrong_token));

  WAIT_EXPECT_TRUE(1000,
                   [&]() -> bool { return response_invoke_failed != nullptr; });
  EXPECT_EQ(response_invoke_failed->get_status(),
            MessageStatus::INVALID_PARAMETER);
  EXPECT_FALSE(linkResp->is_destroyed());

  auto close_request_with_valid_token = make_ref_<InvokeRequestMessage>();
  close_request_with_valid_token->set_value(Var(close_token));
  close_request_with_valid_token->set_target_path("sys/stop");

  link->invoke([&](IncomingInvokeStream &stream,
                   ref_<const InvokeResponseMessage> &&msg) {},
               copy_ref_(close_request_with_valid_token));

  WAIT_EXPECT_TRUE(2000, [&]() -> bool { return linkResp->is_destroyed(); });

  destroy_dslink_in_strand(link);

  app->close();
  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();
}

TEST_F(DslinkTest, ProfileActionTest) {
  shared_ptr<App> app = make_shared<App>();

  const char *argv[] = {"./testResp", "--broker",      "ds://127.0.0.1:4121",
                        "-l",         "info",          "--thread",
                        "4",          "--server-port", "4121"};
  int argc = 9;
  auto link = make_ref_<DsLink>(argc, argv, "mydslink", "1.0.0", app);
  // filter log for unit test
  static_cast<ConsoleLogger &>(link->strand->logger()).filter =
      Logger::FATAL_ | Logger::ERROR_ | Logger::WARN__;

  ref_<NodeModel> profile_example =
      make_ref_<NodeModel>(link->strand->get_ref());
  profile_example->add_list_child(
      "change",
      make_ref_<SimpleInvokeNode>(
          link->strand->get_ref(),
          [&](Var &&v, SimpleInvokeNode &node, OutgoingInvokeStream &stream,
              ref_<NodeState> &&parent) {
            auto *parent_model = parent->model_cast<NodeModel>();
            if (parent_model != nullptr) {
              parent_model->set_value(std::move(v));
            }
            stream.close();
          }));
  link->add_to_pub("example", profile_example->get_ref());

  ref_<NodeModel> main_node =
      make_ref_<NodeModel>(link->strand->get_ref(), profile_example);
  link->init_responder(std::move(main_node));

  bool list_checked = false;
  bool invoked = false;
  bool subscrib_checked = false;
  link->connect([&](const shared_ptr_<Connection> connection) {

    // check the list result
    link->list("main",
               [&](IncomingListCache &cache, const std::vector<string_> &) {
                 if (cache.get_map().count("$is") > 0 &&
                     cache.get_map().at("$is").to_string() == "example") {
                   list_checked = true;
                   cache.close();
                 }
               });
    // invoke the pub node to change the value
    auto request = make_ref_<InvokeRequestMessage>();
    request->set_target_path("main/change");
    request->set_body(Var("hello").to_msgpack());
    link->invoke(
        [&](IncomingInvokeStream &, ref_<const InvokeResponseMessage> &&msg) {
          EXPECT_EQ(msg->get_status(), MessageStatus::CLOSED);
          invoked = true;
        },
        std::move(request));
    // subscribe to check the result
    ref_<IncomingSubscribeCache> sub_cache;
    sub_cache =
        link->subscribe("main", [&](IncomingSubscribeCache &cache,
                                    ref_<const SubscribeResponseMessage> &msg) {
          if (msg->get_value().value.to_string() == "hello") {
            subscrib_checked = true;
            cache.close();
          }
        });

  });

  WAIT_EXPECT_TRUE(1000, [&]() -> bool {
    return list_checked && invoked && subscrib_checked;
  });

  destroy_dslink_in_strand(link);

  app->close();
  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();
}