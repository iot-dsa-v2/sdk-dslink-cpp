#include "dsa/message.h"
#include "dsa/network.h"

#include "../test/sdk/async_test.h"
#include "../test/sdk/test_config.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"

#include <chrono>
#include <ctime>

#include <atomic>

#include <boost/program_options.hpp>

using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

using namespace dsa;
namespace opts = boost::program_options;

class TestConfigExt : public TestConfig {
 public:
  TestConfigExt(App &app, std::string host_ip_address, bool async = false)
      : TestConfig(app, async) {
    tcp_host = host_ip_address;
  }
};

class MockNode : public NodeModelBase {
 public:
  explicit MockNode(LinkStrandRef strand) : NodeModelBase(std::move(strand)){};
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

  App app(num_thread);

  std::cout << std::endl << "host ip address: " << host_ip_address;
  std::cout << std::endl
            << "benchmark with " << client_count << " clients (" << num_thread
            << " threads)";

  TestConfigExt server_config(app, host_ip_address);

  MockNode *root_node = new MockNode(server_config.strand);

  server_config.get_link_config()->set_responder_model(
      ref_<MockNode>(root_node));

  //  auto tcp_server(new TcpServer(server_config));

  std::vector<WrapperConfig> client_configs;
  std::vector<shared_ptr_<Client>> clients;
  std::atomic_int receive_count[MAX_CLIENT_COUNT];

  SubscribeOptions initial_options;
  initial_options.qos = QosLevel::_1;
  initial_options.queue_size = 655360;

  for (int i = 0; i < client_count; ++i) {
    client_configs.emplace_back(server_config.get_client_config(app));
    clients.emplace_back(make_shared_<Client>(client_configs[i]));
    clients[i]->connect();

    wait_for_bool(500, *client_configs[i].strand,
                  [&]() { return clients[i]->get_session().is_connected(); });

    if (!clients[i]->get_session().is_connected()) {
      std::cout << std::endl
                << "client " << i + 1 << " is NOT connected" << std::endl;
      return 1;
    }
    std::cout << std::endl
              << "client " << i + 1 << " is connected" << std::endl;

    receive_count[i] = 0;

    std::atomic_int &count = receive_count[i];

    clients[i]->get_session().requester.subscribe(
        "",
        [&](IncomingSubscribeStream &stream,
            ref_<const SubscribeResponseMessage> &&msg) {
          count.fetch_add(1);
          if (decode_value) {
            msg->get_value();
          }
        },
        initial_options);

    std::cout << "client " << i + 1 << " submitted a subscribe request"
              << std::endl;
  }

  int64_t msg_per_second = 300000;

  boost::posix_time::milliseconds interval(10);
  boost::asio::deadline_timer timer(app.io_service(), interval);

  int print_count = 0;  // print every 100 timer visit;

  auto ts = high_resolution_clock::now();
  int total_ms = 0;

  int total_message = 0;

  SubscribeResponseMessageCRef cached_message =
      make_ref_<SubscribeResponseMessage>(Var(0));

  std::function<void(const boost::system::error_code &)> tick;
  tick = [&](const boost::system::error_code &error) {

    if (!error) {
      client_configs[0].strand->dispatch([&]() {
        auto ts2 = high_resolution_clock::now();

        auto ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(ts2 - ts)
                .count();

        if (ms > 0) {
          ts = ts2;
          int count = 0;
          for (int i = 0; i < client_count; ++i) {
            count += receive_count[i];
            receive_count[i] = 0;
          }
          total_message += count;

          count /= client_count;

          print_count += ms;
          if (print_count > 2000) {
            print_count = 0;
            std::cout << std::endl
                      << "message per second: "
                      << (msg_per_second * client_count)
                      << "  current: " << count * 1000 / ms << " x "
                      << client_count << ", interval " << ms;
          }
        }
        timer.async_wait(tick);
      });
    }
  };

  timer.async_wait(tick);

  std::cout << std::endl << "run benchmark for " << run_time << " seconds";
  boost::this_thread::sleep(boost::posix_time::seconds(run_time));
  std::cout << std::endl << "end benchmark";
  timer.cancel();
  std::cout << std::endl << "total message: " << total_message;

  for (int i = 0; i < client_count; ++i) {
    Client::destroy_in_strand(clients[i]);
  }

  app.close();

  wait_for_bool(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
  return 0;
}
