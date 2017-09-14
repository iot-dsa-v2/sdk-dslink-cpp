#include "dsa/message.h"
#include "dsa/network.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

#include "network/tcp/tcp_client.h"
#include "network/tcp/tcp_server.h"

using namespace dsa;

class MockNode : public NodeModel {
 public:
  std::unique_ptr<SubscribeOptions> first_subscribe_options;
  std::unique_ptr<SubscribeOptions> second_subscribe_options;

  explicit MockNode(LinkStrandRef strand) : NodeModel(std::move(strand)){};

  void on_subscribe(const SubscribeOptions &options) override {
    first_subscribe_options.reset(new SubscribeOptions(options));
    if (_subscribe_callback != nullptr) {
      set_value(Variant("hello"));
    }
  }
  void on_subscribe_option_change(const SubscribeOptions &options) override {
    second_subscribe_options.reset(new SubscribeOptions(options));
    if (_subscribe_callback != nullptr) {
      set_value(Variant("world"));
    }
  }
};

TEST(ResponderTest, Subscribe) {
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

  ASYNC_EXPECT_TRUE(500, (*client_config.strand)(),
                    [&]() { return tcp_client->get_session().is_connected(); });

  SubscribeOptions initial_options;
  initial_options.queue_time = 0x1234;
  initial_options.queue_size = 0x5678;

  SubscribeOptions update_options;
  update_options.queue_time = 0x9876;
  update_options.queue_size = 0x5432;

  ref_<const SubscribeResponseMessage> last_response;
  auto subscribe_stream = tcp_client->get_session().requester.subscribe(
      "",
      [&](ref_<const SubscribeResponseMessage> &&msg,
          IncomingSubscribeStream &stream) {
        last_response = std::move(msg);  // store response
      },
      initial_options);

  // wait for root_node to receive the request
  WAIT_EXPECT_TRUE(500, [&]() -> bool {
    return root_node->first_subscribe_options != nullptr;
  });

  // received request option should be same as the original one
  EXPECT_TRUE(initial_options == *root_node->first_subscribe_options);

  WAIT_EXPECT_TRUE(500, [&]() -> bool { return last_response != nullptr; });

  EXPECT_TRUE(last_response->get_value().has_value() &&
              last_response->get_value().value.is_string() &&
              last_response->get_value().value.get_string() == "hello");

  // send an new request to update the option of the same stream
  auto second_request = make_ref_<SubscribeRequestMessage>();
  second_request->set_subscribe_option(update_options);
  subscribe_stream->send_message(
      std::move(second_request));  // send update options

  WAIT_EXPECT_TRUE(500, [&]() -> bool {
    return root_node->second_subscribe_options != nullptr;
  });
  // request option should be same as the second one
  EXPECT_TRUE(update_options == *root_node->second_subscribe_options);

  WAIT_EXPECT_TRUE(500, [&]() -> bool {
    return last_response->get_value().value.get_string() == "world";
  });


  Server::close_in_strand(tcp_server);
  Client::close_in_strand(tcp_client);

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
