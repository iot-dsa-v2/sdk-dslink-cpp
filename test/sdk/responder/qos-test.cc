#include "dsa/message.h"
#include "dsa/network.h"
#include "dsa/stream.h"

#include <chrono>

#include <gtest/gtest.h>
#include <module/logger.h>
#include "../async_test.h"
#include "../test_config.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"
#include "module/logger.h"
using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

using namespace dsa;

class MockNodeQos : public NodeModelBase {
 public:
  explicit MockNodeQos(LinkStrandRef strand)
      : NodeModelBase(std::move(strand)){};

  void on_subscribe(const SubscribeOptions &options,
                    bool first_request) override {
    if (_need_subscribe) {
      for (int i = 0; i < 10; ++i) {
        set_value(Var(i));
      }
    }
  }
};

TEST(ResponderTest, QosQueueSizeTest) {
  auto app = std::make_shared<App>();

  TestConfig server_strand(app);

  MockNodeQos *root_node = new MockNodeQos(server_strand.strand);

  server_strand.strand->set_responder_model(ref_<MockNodeQos>(root_node));

  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();

  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  SubscribeOptions initial_options;
  initial_options.qos = QosLevel::_1;
  initial_options.queue_size = 150;  // 150 bytes

  ref_<const SubscribeResponseMessage> last_response;
  size_t msg_count = 0;
  auto subscribe_stream = tcp_client->get_session().requester.subscribe(
      "",
      [&](IncomingSubscribeStream &stream,
          ref_<const SubscribeResponseMessage> &&msg) {
        ++msg_count;
        last_response = std::move(msg);  // store response
      },
      initial_options);

  // wait for root_node to receive the request
  ASYNC_EXPECT_TRUE(1000, *client_strand.strand, [&]() -> bool {
    return last_response != nullptr && last_response->get_value().has_value() &&
           last_response->get_value().value.is_int() &&
           last_response->get_value().value.get_int() == 9;
  });

  // can't receive all message because queue size limit
  EXPECT_LE(msg_count, 3);

  tcp_server->destroy_in_strand(tcp_server);
  destroy_client_in_strand(tcp_client);

  server_strand.destroy();
  client_strand.destroy();
  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();
}
