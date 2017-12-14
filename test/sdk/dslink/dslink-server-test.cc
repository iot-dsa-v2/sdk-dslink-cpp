#include <util/string.h>
#include "dsa/network.h"
#include "dsa/responder.h"
#include "dsa/stream.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

#include "message/request/invoke_request_message.h"


using namespace dsa;
using namespace std;

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


TEST(DSLinkTest, Server_Test) {
  shared_ptr<App> app = make_shared<App>();

  const char
      *argv[] = {"./testResp", "--broker", "ds://127.0.0.1:4120", "-l", "info", "--thread", "4", "--server-port", "4121"};
  int argc = 9;
  auto linkResp = make_ref_<DsLink>(argc, argv, "mydslink", "1.0.0");
  linkResp->init_responder<ExampleNodeRoot>();
  linkResp->connect([&](const shared_ptr_<Connection> connection) {});

// Create link
  std::string address =
      std::string("127.0.0.1:") + std::to_string(4121);

  const char *argv2[] = {"./test", "-b", address.c_str()};
  int argc2 = 3;
  auto link = make_ref_<DsLink>(argc2, argv2, "mydslink", "1.0.0", app);

// connection
  bool is_connected = false;
  link->connect([&](const shared_ptr_<Connection> connection) { is_connected = true; });

  ASYNC_EXPECT_TRUE(500, *link->strand, [&]() { return is_connected; });

  std::vector<string_> list_result;
  // List test
  link->list("",
             [&](IncomingListCache &cache, const std::vector<string_> &str) {
               list_result = str;
             });
  ASYNC_EXPECT_TRUE(500, *link.get()->strand,
                    [&]() { return list_result.size() == 3; });

  EXPECT_CONTAIN(list_result, "sys");
  EXPECT_CONTAIN(list_result, "pub");
  EXPECT_CONTAIN(list_result, "main");

// add a callback when connected to broker
  std::vector<std::string> messages;
  link->subscribe("main/child_a",
                  [&](IncomingSubscribeCache &cache, ref_<const SubscribeResponseMessage> message) {
                    messages.push_back(message->get_value().value.get_string());
                  });

  ASYNC_EXPECT_TRUE(500, *link.get()->strand,
                    [&]() { return messages.size() == 1; });

  EXPECT_TRUE(messages.size() == 1);
  EXPECT_EQ(messages.at(0), "test string value 1");

// Cleaning test
  destroy_dslink_in_strand(linkResp);
  destroy_dslink_in_strand(link);

  app->close();
  WAIT_EXPECT_TRUE(500, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();

}

TEST(DSLinkTest, CloseTest) {
  shared_ptr<App> app = make_shared<App>();

  // first create .close_token
  string_ close_token = "12345678901234567890123456789012";
  try{
    close_token = string_from_file(".close_token");
  }catch(std::exception &e){
    string_to_file(close_token, ".close_token");
  }

  const char
      *argv[] = {"./testResp", "--broker", "ds://127.0.0.1:4120", "-l", "info", "--thread", "4", "--server-port", "4121"};
  int argc = 9;
  auto linkResp = make_ref_<DsLink>(argc, argv, "mydslink", "1.0.0");
  linkResp->init_responder<ExampleNodeRoot>();
  linkResp->connect([&](const shared_ptr_<Connection> connection) {});

  EXPECT_EQ(close_token, linkResp->get_close_token());

// Create link
  std::string address =
      std::string("127.0.0.1:") + std::to_string(4121);

  const char *argv2[] = {"./test", "-b", address.c_str()};
  int argc2 = 3;
  auto link = make_ref_<DsLink>(argc2, argv2, "mydslink", "1.0.0", app);

// connection
  bool is_connected = false;
  link->connect([&](const shared_ptr_<Connection> connection) { is_connected = true; });

  ASYNC_EXPECT_TRUE(500, *link->strand, [&]() { return is_connected; });

  auto close_request_with_wrong_token = make_ref_<InvokeRequestMessage>();
  close_request_with_wrong_token->set_value(Var("wrongtoken"));
  close_request_with_wrong_token->set_target_path("sys/stop");

  link->invoke( [&](IncomingInvokeStream &stream,
                    ref_<const InvokeResponseMessage> &&msg) {
                },
                copy_ref_(close_request_with_wrong_token));

  WAIT(1000);
  EXPECT_FALSE(linkResp->is_destroyed());

  auto close_request_with_valid_token = make_ref_<InvokeRequestMessage>();
  close_request_with_wrong_token->set_value(Var(close_token));
  close_request_with_wrong_token->set_target_path("sys/stop");

  link->invoke( [&](IncomingInvokeStream &stream,
                    ref_<const InvokeResponseMessage> &&msg) {
                },
                copy_ref_(close_request_with_wrong_token));

  ASYNC_EXPECT_TRUE(1000, *linkResp->strand, [&]() { return linkResp->is_destroyed(); });

  destroy_dslink_in_strand(link);

  app->close();
  WAIT_EXPECT_TRUE(500, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();

}

