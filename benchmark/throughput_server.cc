#include "throughput_common.h"

int main(int argc, const char *argv[]) {
  opts::options_description desc{"Options"};
  desc.add_options()("help,h", "Help screen")(
      "client,c", opts::value<int>()->default_value(2), "Number of Clients")(
      "time,t", opts::value<int>()->default_value(20), "Time (seconds)")(
      "encode-value,e", opts::bool_switch(), "Encode value before sending")(
      "decode-value,d", opts::bool_switch(), "Decode value after receiving")(
      "num-message,n", opts::value<int>()->default_value(5000),
      "Minimal number of messages to send in each iteration")(
      "host,i", opts::value<std::string>()->default_value("127.0.0.1"),
      "Host's ip address")("port,p", opts::value<int>()->default_value(4128),
                           "Port")(
      "num-thread", opts::value<int>()->default_value(4), "Number of threads");

  opts::variables_map variables;
  opts::store(opts::parse_command_line(argc, argv, desc), variables);
  opts::notify(variables);

  if (variables.count("help")) {
    std::cout << desc << '\n';
    return 0;
  }

  const int MAX_CLIENT_COUNT = 256;
  int client_count = variables["client"].as<int>();
  if (client_count > MAX_CLIENT_COUNT || client_count < 1) {
    std::cout << "invalid Number of Clients, ( 1 ~ 255 )";
    return 0;
  }

  int run_time = variables["time"].as<int>();
  bool encode_value = variables["encode-value"].as<bool>();
  bool decode_value = variables["decode-value"].as<bool>();
  int min_send_num = variables["num-message"].as<int>();
  int host_port = variables["port"].as<int>();
  std::string host_ip_address = variables["host"].as<std::string>();
  int num_thread = variables["num-thread"].as<int>();

  Logger::_().level = Logger::INFO__;

  std::cout << std::endl << "host ip address: " << host_ip_address << std::endl;
  std::cout << std::endl << "host port: " << host_port << std::endl;
  std::cout << "benchmark with " << client_count << " clients (" << num_thread
            << " threads)" << std::endl;

  const int MAX_NUM_MSG_PER_CLIENT = 10;

  MessageQueue sc_mq(create_only, sc_mq_name,
                     client_count * MAX_NUM_MSG_PER_CLIENT, sizeof(int32_t),
                     client_count);

  MessageQueues inbound_mqs(cs_mq_name_base,
                     MAX_NUM_MSG_PER_CLIENT, sizeof(int32_t),
                     client_count);

  auto app = std::make_shared<App>(num_thread);

  TestConfigExt server_strand(app, host_ip_address, host_port);

  MockNode *root_node = new MockNode(server_strand.strand);

  server_strand.strand->set_responder_model(ref_<MockNode>(root_node));

  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  // wait for all clients connect to the server
  inbound_mqs.gather();
  // starts responsing to subscribe requests
  sc_mq.scatter();

  int64_t msg_per_second = 300000;

  int print_count = 0;  // print every 100 timer visit;

  auto ts = high_resolution_clock::now();
  auto ts_then = ts;

  int total_ms = 0;

  int total_message = 0;

  uint64_t count = 0;

  SubscribeResponseMessageCRef cached_message =
      make_ref_<SubscribeResponseMessage>(Var(0));

  do {
    auto ts_now = high_resolution_clock::now();
    auto ms_delta =
        std::chrono::duration_cast<std::chrono::milliseconds>(ts_now - ts_then)
            .count();
    if (ms_delta > run_time * 1000) break;

    if (server_strand.strand == nullptr) break;

    server_strand.strand->dispatch([&]() {
      auto ts2 = high_resolution_clock::now();

      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(ts2 - ts)
                    .count();

      if (ms > 0) {
        ts = ts2;

        total_message += count;

        print_count += ms;
        if (print_count > 2000) {
          print_count = 0;
          std::cout << std::endl
                    << "message per second: " << (msg_per_second * client_count)
                    << "  current: " << count * 1000 / ms << " x "
                    << client_count << ", interval " << ms;
        }

        msg_per_second =
            (count * 1000 + msg_per_second * total_ms) / (total_ms + ms);
        total_ms += ms / 2;
        if (total_ms > 5000) total_ms = 5000;

        long tosend_per_second = msg_per_second;
        if (tosend_per_second < min_send_num) tosend_per_second = min_send_num;
        // send a little bit more than the current speed,
        // limited message queue size should handle the extra messages
        long num_message = tosend_per_second * ms / (800 + total_ms / 50);

        if (encode_value) {
          for (int i = 0; i < num_message; ++i) {
            root_node->set_value(Var(i));
          }
        } else {
          for (int i = 0; i < num_message; ++i) {
            root_node->set_subscribe_response(copy_ref_(cached_message));
          }
        }
      }
    });
    // TODO: timeout version is needed
    count = 0;
    inbound_mqs.gather(count);
  } while (true);

  std::cout << std::endl
            << "run benchmark for " << run_time << " seconds" << std::endl;

  tcp_server->destroy_in_strand(tcp_server);
  server_strand.destroy();

  app->close();

  wait_for_bool(500, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();

  return 0;
}
