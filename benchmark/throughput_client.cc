#include "throughput_common.h"

int main(int argc, const char *argv[]) {
  opts::options_description desc{"Options"};
  desc.add_options()("help,h", "Help screen")(
      "time,t", opts::value<int>()->default_value(20), "Time (seconds)")(
      "encode-value,e", opts::bool_switch(), "Encode value before sending")(
      "decode-value,d", opts::bool_switch(), "Decode value after receiving")(
      "num-message,n", opts::value<int>()->default_value(5000),
      "Minimal number of messages to send in each iteration")(
      "host,i", opts::value<std::string>()->default_value("10.0.1.101"),
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

  int run_time = variables["time"].as<int>();
  bool encode_value = variables["encode-value"].as<bool>();
  bool decode_value = variables["decode-value"].as<bool>();
  int min_send_num = variables["num-message"].as<int>();
  int host_port = variables["port"].as<int>();
  std::string host_ip_address = variables["host"].as<std::string>();
  int num_thread = variables["num-thread"].as<int>();

  Logger::_().level = Logger::INFO__;

  auto app = std::make_shared<App>(num_thread);

  std::cout << std::endl << "host ip address: " << host_ip_address;
  std::cout << std::endl
            << "benchmark with " << num_thread << " threads)";

  TestConfigExt server_strand(app, host_ip_address, host_port);

  MockNode *root_node = new MockNode(server_strand.strand);

  server_strand.strand->set_responder_model(ref_<MockNode>(root_node));

  SubscribeOptions initial_options;
  initial_options.qos = QosLevel::_1;
  initial_options.queue_size = 655360;

  MessageQueue sc_mq(open_only, sc_mq_name.c_str());
  MessageQueue cs_mq(open_only, cs_mq_name.c_str());

  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();
  ref_<Client> client = make_ref_<Client>(client_strand);
  std::atomic_int receive_count;
  client->connect();

  wait_for_bool(500, *client_strand.strand,
                [&]() { return client->get_session().is_connected(); });

  if (!client->get_session().is_connected()) {
    std::cout << std::endl
              << "client is NOT connected" << std::endl;
    return 1;
  }
  std::cout << std::endl << "client is connected" << std::endl;

  cs_mq.sync();
  sc_mq.wait();

  receive_count = 0;

  std::atomic_int &count = receive_count;

  client->get_session().subscribe(
      "",
      [&](IncomingSubscribeStream &stream,
          ref_<const SubscribeResponseMessage> &&msg) {
        count.fetch_add(1);
        if (decode_value) {
          msg->get_value();
        }
      },
      initial_options);

  std::cout << "client submitted a subscribe request" << std::endl;

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
      if (client_strand.strand == nullptr) return;
      client_strand.strand->dispatch([&]() {
        auto ts2 = high_resolution_clock::now();

        auto ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(ts2 - ts)
                .count();

        if (ms > 0) {
          ts = ts2;

          cs_mq.sync(receive_count);

          int count = 0;
          count = receive_count;
          receive_count = 0;

          total_message += count;

          print_count += ms;
          if (print_count > 2000) {
            print_count = 0;
            std::cout << std::endl
                      << "message per second: "
                      << (msg_per_second * 1)
                      << "  current: " << count * 1000 / ms << " x "
                      << 1 << ", interval " << ms;
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
  std::cout << std::endl << "total message: " << total_message << std::endl;

  destroy_client_in_strand(client);

  server_strand.destroy();
  client_strand.destroy();

  app->close();

  wait_for_bool(500, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  app->wait();

  return 0;
}
