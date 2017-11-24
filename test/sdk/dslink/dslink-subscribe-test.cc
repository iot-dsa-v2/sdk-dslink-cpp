
#include "dsa/network.h"
#include "dsa/responder.h"
#include "dsa/stream.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"
#include "module/logger.h"


#include "dslink.h"
#include "dsa/network.h"
#include "dsa/stream.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"

using namespace dsa;
using namespace std;
using namespace boost::asio;

namespace link_subscribe_test {
class MockNode : public NodeModelBase {
 public:
  std::unique_ptr<SubscribeOptions> first_subscribe_options;
  std::unique_ptr<SubscribeOptions> second_subscribe_options;
  bool need_subscribe() { return _need_subscribe; }

  explicit MockNode(LinkStrandRef strand) : NodeModelBase(std::move(strand)) {};

  void on_subscribe(const SubscribeOptions &options,
                    bool first_request) override {
    if (first_request) {
      first_subscribe_options.reset(new SubscribeOptions(options));
      set_value(Var("hello"));
    } else {
      second_subscribe_options.reset(new SubscribeOptions(options));
      set_value(Var("world"));
    }
  }

};

boost::mutex global_lock;

struct SubscribeCallbackTest {

  static void subscribe_callback(IncomingSubscribeCache &a, ref_<const SubscribeResponseMessage> message,
                                 std::vector<std::string> *messages) {
    global_lock.lock();
    messages->push_back(message->get_value().value.get_string());
    global_lock.unlock();
  };

  static void connection_callback(const shared_ptr_<Connection> connection,
                                  bool* is_connected) {
    *is_connected = true;
  };
};

}

TEST(DSLinkTest, Subscribe_Test) {
  typedef link_subscribe_test::MockNode MockNode;
  typedef link_subscribe_test::SubscribeCallbackTest SubscribeCallbackTest;

  shared_ptr<App> app = make_shared<App>();
  TestConfig server_strand(app);
  MockNode *root_node = new MockNode(server_strand.strand);
  server_strand.strand->set_responder_model(ModelRef(root_node));
  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  // Create link
  auto link = server_strand.create_dslink();

  // connection
  bool is_connected = false;
  link->connect(std::bind(SubscribeCallbackTest::connection_callback, std::placeholders::_1, &is_connected));
  ASYNC_EXPECT_TRUE(500, *link->strand, [&]() {return is_connected;});

  // add a callback when connected to broker
  std::vector<std::string> messages;

  link->subscribe("", std::bind(SubscribeCallbackTest::subscribe_callback,
                                std::placeholders::_1, std::placeholders::_2,
                                &messages));
  WAIT(500);

  EXPECT_TRUE(messages.size() == 1);
  EXPECT_EQ(messages.at(0), "hello");

  // Cleaning test
  tcp_server->destroy_in_strand(tcp_server);
  destroy_dslink_in_strand(link);

  app->close();
  WAIT(500);

  if (!app->is_stopped()) {
    app->force_stop();
  }

  server_strand.destroy();

  app->wait();

};

TEST(LinkTest, Subscribe_Multi_Test) {
  typedef link_subscribe_test::MockNode MockNode;
  typedef link_subscribe_test::SubscribeCallbackTest SubscribeCallbackTest;

  auto app = make_shared<App>();
  TestConfig server_strand(app);
  MockNode *root_node = new MockNode(server_strand.strand);
  server_strand.strand->set_responder_model(ModelRef(root_node));
  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  // Create link
  auto link = server_strand.create_dslink();

  // connection
  bool is_connected = false;
  link->connect(std::bind(SubscribeCallbackTest::connection_callback, std::placeholders::_1, &is_connected));
  ASYNC_EXPECT_TRUE(500, *link->strand, [&]() {return is_connected;});

  // Initial Subcribe
  std::vector<std::string> messages_initial;

  SubscribeOptions initial_options;
  initial_options.queue_duration = 0x1234;
  initial_options.queue_size = 0x5678;

  link->subscribe("", std::bind(SubscribeCallbackTest::subscribe_callback,
                                std::placeholders::_1, std::placeholders::_2,
                                &messages_initial), initial_options);
  WAIT(500);

  // Subscribe update
  std::vector<std::string> messages_updated;

  SubscribeOptions update_options;
  update_options.queue_duration = 0x9876;
  update_options.queue_size = 0x5432;

  link->subscribe("", std::bind(SubscribeCallbackTest::subscribe_callback,
                                std::placeholders::_1, std::placeholders::_2,
                                &messages_updated), update_options);
  WAIT(500);

  // TODO: How to Check return messages
  EXPECT_TRUE(messages_initial.size() != 0);
  for( auto it=messages_initial.begin(); it != messages_initial.end(); it++)
    EXPECT_EQ(*it, "hello");

  EXPECT_TRUE(messages_updated.size() != 0);
  for( auto it=messages_updated.begin(); it != messages_updated.end(); it++)
    EXPECT_EQ(*it, "world");

  // Cleaning test
  tcp_server->destroy_in_strand(tcp_server);
//  destroy_dslink_in_strand(link);

  app->close();
  WAIT(500);

  if (!app->is_stopped()) {
    app->force_stop();
  }

  server_strand.destroy();

//  app->wait();
  link->run();
};

