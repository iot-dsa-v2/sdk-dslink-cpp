#include "dsa/network.h"
#include "dsa/responder.h"
#include "dsa/stream.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

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
  linkResp->init_responder_main<ExampleNodeRoot>();
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

// add a callback when connected to broker
  std::vector<std::string> messages;

  link->subscribe("child_a",
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