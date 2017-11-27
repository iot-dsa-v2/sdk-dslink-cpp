#include "dsa/message.h"
#include "dsa/stream.h"
#include "dsa/network.h"

#include "../test/sdk/async_test.h"
#include "../test/sdk/test_config.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"

#include <chrono>
#include <ctime>

#include <atomic>

#include <boost/program_options.hpp>

#include <iostream>

using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

using namespace dsa;
namespace opts = boost::program_options;

class TestConfigExt : public TestConfig {
 public:
  TestConfigExt(std::shared_ptr<App> app, std::string host_ip_address, bool async = false)
      : TestConfig(app, async) {
    tcp_host = host_ip_address;
  }
};

class MockNode : public NodeModelBase {
 public:
  bool first_client_subscribed = false;

  explicit MockNode(LinkStrandRef strand) : NodeModelBase(std::move(strand)){};

  void on_subscribe(const SubscribeOptions &options, bool first_request) override {
    first_client_subscribed = true;
  }
};

int main(int argc, const char *argv[]) {
  opts::options_description desc{"Options"};
  desc.add_options()("help,h", "Help screen")(
      "client,c", opts::value<int>()->default_value(2), "Number of Clients")(
      "time,t", opts::value<int>()->default_value(20), "Time (seconds)")(
      "encode-value,e", opts::bool_switch(), "Encode value before sending")(
      "decode-value,d", opts::bool_switch(), "Decode value after receiving")(
      "num-message,n", opts::value<int>()->default_value(5000),
      "Minimal number of messages to send in each iteration")(
      "host,i", opts::value<std::string>()->default_value("10.0.1.101"),
      "Host's ip address")("num-thread,p", opts::value<int>()->default_value(4),
                           "Number of threads");

  opts::variables_map variables;
  opts::store(opts::parse_command_line(argc, argv, desc), variables);
  opts::notify(variables);

  if (variables.count("help")) {
    std::cout << desc << '\n';
    return 0;
  }

  const int MAX_CLIENT_COUNT = 256;

  int client_count = variables["client"].as<int>();
  int run_time = variables["time"].as<int>();
  std::string host_ip_address = variables["host"].as<std::string>();

  if (client_count > MAX_CLIENT_COUNT || client_count < 1) {
    std::cout << "invalid Number of Clients, ( 1 ~ 255 )";
    return 0;
  }
  bool encode_value = variables["encode-value"].as<bool>();
  bool decode_value = variables["decode-value"].as<bool>();
  int min_send_num = variables["num-message"].as<int>();
  int num_thread = variables["num-thread"].as<int>();

  std::cout << std::endl << "host ip address: " << host_ip_address << std::endl;
  std::cout << "benchmark with " << client_count << " clients (" << num_thread
            << " threads)" << std::endl;

  auto app = std::make_shared<App>(num_thread);

  TestConfigExt server_strand(app, host_ip_address);

  MockNode *root_node = new MockNode(server_strand.strand);

  server_strand.strand->set_responder_model(
      ref_<MockNode>(root_node));

  //  auto tcp_server(new TcpServer(server_strand));
  auto tcp_server = make_shared_<TcpServer>(server_strand);
  tcp_server->start();

  wait_for_bool(5000,
                [&]() { return root_node->first_client_subscribed == true; });
  if (!root_node->first_client_subscribed) {
    std::cout << "no subscribe request!" << std::endl;
    return 1;
  }

  std::cout << "received first subscribe request" << std::endl;

  int64_t msg_per_second = 300000;

  boost::posix_time::milliseconds interval(10);
  boost::asio::deadline_timer timer(app->io_service(), interval);

  auto ts = high_resolution_clock::now();
  int total_ms = 0;

  int total_message = 0;

  SubscribeResponseMessageCRef cached_message =
      make_ref_<SubscribeResponseMessage>(Var(0));

  std::function<void(const boost::system::error_code &)> tick;
  tick = [&](const boost::system::error_code &error) {

    if (!error) {
      server_strand.strand->dispatch([&]() {
        auto ts2 = high_resolution_clock::now();

        auto ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(ts2 - ts)
                .count();

        if (ms > 0) {
          ts = ts2;

          // TODO: adjust dynamically
          long num_message = 6000;

          if (encode_value) {
            for (int i = 0; i < num_message; ++i) {
              root_node->set_value(Var(i));
            }
          } else {
            for (int i = 0; i < num_message; ++i) {
              root_node->set_message(copy_ref_(cached_message));
            }
          }
        }
        timer.async_wait(tick);
      });
    } else {
      std::cout << "tick: error!" << std::endl;
    }
  };

  timer.async_wait(tick);

  boost::this_thread::sleep(boost::posix_time::seconds(run_time));
  timer.cancel();

  tcp_server->destroy_in_strand(tcp_server);

  app->close();

  wait_for_bool(500, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  server_strand.destroy();
  app->wait();

  return 0;
}
