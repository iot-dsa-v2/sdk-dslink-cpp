
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
                                  DsLink* link, std::vector<std::string> *messages) {
    link->subscribe("", std::bind(SubscribeCallbackTest::subscribe_callback,
                                  std::placeholders::_1, std::placeholders::_2,
                                  messages));
  };

  static void resubscribe_timer_callback(const boost::system::error_code &error,
                                         DsLink* link,
                                         std::vector<std::string> *messages)
  {
    SubscribeOptions updated_options;
    updated_options.queue_duration = 0x9876;
    updated_options.queue_size = 0x5432;

    link->subscribe("", std::bind(SubscribeCallbackTest::subscribe_callback,
                                  std::placeholders::_1, std::placeholders::_2,
                                  messages), updated_options);
  }


  static void connection_callback_multi_subs(const shared_ptr_<Connection> connection,
                                             DsLink* link,
                                             std::vector<std::string> *messages_first,
                                             std::vector<std::string> *messages_updated,
                                             shared_ptr<deadline_timer> timer) {



    link->subscribe("", std::bind(SubscribeCallbackTest::subscribe_callback,
                                  std::placeholders::_1, std::placeholders::_2,
                                  messages_first));

    // second subscribe will be timed
    timer->expires_from_now(boost::posix_time::seconds(1));
    timer->async_wait(std::bind(resubscribe_timer_callback,  std::placeholders::_1, link, messages_updated));

  };
};

void test_time_out(const boost::system::error_code &error, App& app) {
  global_lock.lock();
  app.force_stop();
  global_lock.unlock();
};

ref_<DsLink> get_test_dslink()
{
  const char *argv[] = {"./test", "-b", "127.0.0.1:4120"};
  int argc = 3;
  auto link = make_ref_<DsLink>(argc, argv, "mydslink", "1.0.0");
  link->init_responder<MockNode>();

  return link;
};

shared_ptr<deadline_timer> get_deadline_timer(boost::asio::io_service& io_service, float seconds = -1.0 ){
  shared_ptr<deadline_timer> timer = make_shared<deadline_timer>(io_service);
  if(seconds > 0.0)
    timer->expires_from_now(boost::posix_time::seconds(seconds));
  return timer;
}

}
//
//TEST(LinkTest, Subscribe_Test) {
//  typedef link_subscribe_test::MockNode MockNode;
//  typedef link_subscribe_test::SubscribeCallbackTest SubscribeCallbackTest;
//
//  App app;
//  TestConfig server_strand(app);
//  MockNode *root_node = new MockNode(server_strand.strand);
//  server_strand.strand->set_responder_model(ModelRef(root_node));
//  auto tcp_server = make_shared_<TcpServer>(server_strand);
//  tcp_server->start();
//
//  // Create link and its quit timer
//  auto link = link_subscribe_test::get_test_dslink();
//  auto quit_timer = link_subscribe_test::get_deadline_timer(link->get_app().io_service(), 2.0 );
//  quit_timer->async_wait(std::bind(link_subscribe_test::test_time_out, std::placeholders::_1, link->get_app()));
//
//  // add a callback when connected to broker
//  std::vector<std::string> messages;
//  link->run(std::bind(SubscribeCallbackTest::connection_callback,
//                      std::placeholders::_1, link.get(), &messages));
//
//  EXPECT_FALSE(messages.size() == 0);
//  EXPECT_EQ(messages.at(0), "hello");
//
//  // Cleaning test
//  tcp_server->destroy_in_strand(tcp_server);
//  link->destroy();
//
//  if (!app.is_stopped()) {
//    app.force_stop();
//  }
//
//  server_strand.destroy();
//
//  app.close();
//
//};

TEST(LinkTest, Subscribe_Multi_Test) {
  typedef link_subscribe_test::MockNode MockNode;
  typedef link_subscribe_test::SubscribeCallbackTest SubscribeCallbackTest;

  App app;
  TestConfig server_strand(app);
  MockNode *root_node = new MockNode(server_strand.strand);
  server_strand.strand->set_responder_model(ModelRef(root_node));
  auto tcp_server = make_shared_<TcpServer>(server_strand);
  tcp_server->start();

  // Create link and its quit timer
  auto link = link_subscribe_test::get_test_dslink();
  auto quit_timer = link_subscribe_test::get_deadline_timer(link->get_app().io_service(), 5.0 );
  quit_timer->async_wait(std::bind(link_subscribe_test::test_time_out, std::placeholders::_1, link->get_app()));

  // add a callback when connected to broker
  std::vector<std::string> messages_first;
  std::vector<std::string> messages_updated;
  auto timer = link_subscribe_test::get_deadline_timer(link->get_app().io_service());
  link->run(std::bind(SubscribeCallbackTest::connection_callback_multi_subs,
                      std::placeholders::_1, link.get(), &messages_first, &messages_updated, timer));

  EXPECT_FALSE(messages_first.size() == 0);
  for( auto it=messages_first.begin(); it != messages_first.end(); it++)
    EXPECT_EQ(*it, "hello");

  EXPECT_FALSE(messages_updated.size() == 0);
  for( auto it=messages_updated.begin(); it != messages_updated.end(); it++)
    EXPECT_EQ(*it, "world");

  // Cleaning test
  tcp_server->destroy_in_strand(tcp_server);
  link->destroy();

  if (!app.is_stopped()) {
    app.force_stop();
  }

  server_strand.destroy();

  app.close();

};