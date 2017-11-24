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

using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

using namespace dsa;
namespace opts = boost::program_options;

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
      "Minimal number of messages to send in each iteration");

  opts::variables_map variables;
  opts::store(opts::parse_command_line(argc, argv, desc), variables);
  opts::notify(variables);

  if (variables.count("help")) {
    std::cout << desc << '\n';
    return 0;
  }

  int run_time = 20;
  int client_count = 2;
  const int MAX_CLIENT_COUNT = 256;

  client_count = variables["client"].as<int>();
  run_time = variables["time"].as<int>();

  if (client_count > MAX_CLIENT_COUNT || client_count < 1) {
    std::cout << "invalid Number of Clients, ( 1 ~ 255 )";
    return 0;
  }
  bool encode_value = variables["encode-value"].as<bool>();
  bool decode_value = variables["decode-value"].as<bool>();
  int min_send_num = variables["num-message"].as<int>();

  std::cout << std::endl << "benchmark with " << client_count << " clients";

  auto app = std::make_shared<App>();

  TestConfig server_strand(app);

  MockNode *root_node = new MockNode(server_strand.strand);

  server_strand.strand->set_responder_model(
      ref_<MockNode>(root_node));

  //  auto tcp_server(new TcpServer(server_strand));
  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  std::vector<WrapperStrand> client_strands;
  std::vector<ref_<Client>> clients;
  std::atomic_int receive_count[MAX_CLIENT_COUNT];

  SubscribeOptions initial_options;
  initial_options.qos = QosLevel::_1;
  initial_options.queue_size = 655360;

  for (int i = 0; i < client_count; ++i) {
    client_strands.emplace_back(server_strand.get_client_wrapper_strand());
    clients.emplace_back(make_ref_<Client>(client_strands[i]));
    clients[i]->connect();

    wait_for_bool(500, *client_strands[i].strand,
                  [&]() { return clients[i]->get_session().is_connected(); });

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
  }

  int64_t msg_per_second = 300000;

  boost::posix_time::milliseconds interval(10);
  boost::asio::deadline_timer timer(app->io_service(), interval);

  int print_count = 0;  // print every 100 timer visit;

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

          msg_per_second =
              (count * 1000 + msg_per_second * total_ms) / (total_ms + ms);
          total_ms += ms / 2;
          if (total_ms > 5000) total_ms = 5000;

          long tosend_per_second = msg_per_second;
          if (tosend_per_second < min_send_num)
            tosend_per_second = min_send_num;
          // send a little bit more than the current speed,
          // limited message queue size should handle the extra messages
          long num_message = tosend_per_second * ms / (800 + total_ms / 50);
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
    }
  };

  timer.async_wait(tick);

  std::cout << std::endl << "run benchmark for " << run_time << " seconds";
  boost::this_thread::sleep(boost::posix_time::seconds(run_time));
  std::cout << std::endl << "end benchmark";
  timer.cancel();
  std::cout << std::endl << "total message: " << total_message;

  tcp_server->destroy_in_strand(tcp_server);
  for (int i = 0; i < client_count; ++i) {
    destroy_client_in_strand(clients[i]);
  }

  app->close();

  wait_for_bool(500, [&]() { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  server_strand.destroy();
  for (int i = 0; i < client_count; ++i) {
    client_strands[i].destroy();
  }
  app->wait();
  return 0;
}
