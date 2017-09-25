#include "dsa/message.h"
#include "dsa/network.h"

#include "../test/sdk/async_test.h"
#include "../test/sdk/test_config.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"

#include <chrono>
#include <ctime>

#define WAIT(wait_time, callback) wait_for_bool((wait_time), (callback))
#define ASYNC(wait_time, strand, callback) \
  wait_for_bool((wait_time), (strand), (callback))

using namespace dsa;

const int SLEEP_INTERVAL = 25;
using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

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
};

int main() {
  App app;

  TestConfig server_config(app);

  MockNode *root_node = new MockNode(server_config.strand);

  server_config.get_link_config()->set_responder_model(ref_<MockNode>(root_node));

  WrapperConfig client_config = server_config.get_client_config(app);

  //  auto tcp_server(new TcpServer(server_config));
  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  auto tcp_client = make_shared_<Client>(client_config);
  tcp_client->connect();

  ASYNC(500, *client_config.strand,
        [&]() { return tcp_client->get_session().is_connected(); });

  SubscribeOptions initial_options;
  initial_options.queue_time = 0x1234;
  initial_options.queue_size = 0x5678;

  ref_<const SubscribeResponseMessage> last_response;

  const uint16_t MAX_NUM_MSGS = 0xffff;
  uint16_t idx = 0;
  time_point start_time_point, end_time_point;

  start_time_point = high_resolution_clock::now();
  while (idx < MAX_NUM_MSGS) {
    auto subscribe_stream = tcp_client->get_session().requester.subscribe(
        "",
        [&](ref_<const SubscribeResponseMessage> &&msg,
            IncomingSubscribeStream &stream) {
          if (idx == (MAX_NUM_MSGS - 1)) {
            last_response = std::move(msg);
          }
        },
        initial_options);

    // move out of the loop?
    if (idx == (MAX_NUM_MSGS - 1)) {
      int waited = 0;
      while (waited < MAX_NUM_MSGS * 100) {
        if (last_response != nullptr) {
          end_time_point = high_resolution_clock::now();
          break;
        }

        boost::this_thread::sleep(
            boost::posix_time::milliseconds(SLEEP_INTERVAL));
        waited += SLEEP_INTERVAL;
      }
    }

    idx++;
  }

  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time_point - start_time_point)
                   .count()
            << std::endl;

  Server::close_in_strand(tcp_server);
  Client::close_in_strand(tcp_client);

  app.close();

  WAIT(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
