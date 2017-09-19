#include "dsa/message.h"
#include "dsa/network.h"

#include <chrono>

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

#include "network/tcp/tcp_client.h"
#include "network/tcp/tcp_server.h"

using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

using namespace dsa;

class MockNode : public NodeModel {
 public:
  explicit MockNode(LinkStrandRef strand) : NodeModel(std::move(strand)){};

  void on_subscribe(const SubscribeOptions &options) override {
    if (_subscribe_callback != nullptr) {
      for (int i = 0; i < 10; ++i) {
        set_value(Variant(i));
      }
    }
  }
};

TEST(ResponderQosTest, QueueSizeTest) {
  App app;

  TestConfig server_config(app);

  MockNode *root_node = new MockNode(server_config.strand);

  server_config.get_link_config()->set_stream_acceptor(
      make_unique_<NodeStateManager>(ref_<MockNode>(root_node)));

  WrapperConfig client_config = server_config.get_client_config(app);

  app.async_start(10);

  //  auto tcp_server(new TcpServer(server_config));
  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  auto tcp_client = make_shared_<TcpClient>(client_config);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  SubscribeOptions initial_options;
  initial_options.qos = QosLevel::_1;
  initial_options.queue_size = 150;  // 150 bytes

  ref_<const SubscribeResponseMessage> last_response;
  size_t msg_count = 0;
  auto subscribe_stream = tcp_client->get_session().requester.subscribe(
      "",
      [&](ref_<const SubscribeResponseMessage> &&msg,
          IncomingSubscribeStream &stream) {
        ++msg_count;
        last_response = std::move(msg);  // store response
      },
      initial_options);

  // wait for root_node to receive the request
  WAIT_EXPECT_TRUE(500, [&]() -> bool {
    return last_response != nullptr && last_response->get_value().has_value() &&
           last_response->get_value().value.is_int() &&
           last_response->get_value().value.get_int() == 9;
  });

  // can't receive all message because queue size limit
  EXPECT_EQ(msg_count, 2);

  Server::close_in_strand(tcp_server);
  Client::close_in_strand(tcp_client);

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
