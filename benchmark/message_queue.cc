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
  explicit MockNode(const LinkStrandRef &strand) : NodeModelBase(strand){};
};

int main(int argc, const char *argv[]) {
  opts::options_description desc{"Options"};
  desc.add_options()("help,h", "Help screen")(
      "encode-value,e", opts::bool_switch(), "Encode value before sending")(
      "num-message,n", opts::value<int>()->default_value(1000000),
      "Number of messages to send");

  opts::variables_map variables;
  opts::store(opts::parse_command_line(argc, argv, desc), variables);
  opts::notify(variables);

  if (variables.count("help")) {
    std::cout << desc << '\n';
    return 0;
  }

  bool encode_value = true;  // variables["encode-value"].as<bool>();

  int num_message = variables["num-message"].as<int>();

  auto app = std::make_shared<App>();

  TestConfig server_strand(app);

  MockNode *root_node = new MockNode(server_strand.strand);

  server_strand.strand->set_responder_model(
      ref_<MockNode>(root_node));

  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();
  ref_<Client> client = make_ref_<Client>(client_strand);
  client->connect();

  std::atomic_int receive_count{0};

  SubscribeOptions initial_options;
  initial_options.qos = QosLevel::_1;
  initial_options.queue_size = 655360;

  wait_for_bool(500, *client_strand.strand,
                [&]() { return client->get_session().is_connected(); });

  client->get_session().subscribe(
      "",
      [&](IncomingSubscribeStream &stream,
          ref_<const SubscribeResponseMessage> &&msg) {
        receive_count.fetch_add(1);

      },
      initial_options);

  SubscribeResponseMessageCRef cached_message =
      make_ref_<SubscribeResponseMessage>(Var(0));

  server_strand.strand->dispatch([&]() {
    auto ts = high_resolution_clock::now();

    if (encode_value) {
      for (int i = 0; i < num_message; ++i) {
        root_node->set_value(Var(i));
      }
    } else {
      for (int i = 0; i < num_message; ++i) {
        root_node->set_message(copy_ref_(cached_message));
      }
    }
    auto ts2 = high_resolution_clock::now();

    auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(ts2 - ts).count();

    std::cout << std::endl
              << num_message << " messages processed in  " << ms << " ms";

    tcp_server->destroy_in_strand(tcp_server);
    destroy_client_in_strand(client);
    app->close();

    wait_for_bool(500, [&]() -> bool { return app->is_stopped(); });

    if (!app->is_stopped()) {
      app->force_stop();
    }
  });

  server_strand.destroy();
  client_strand.destroy();
  app->wait();
  return 0;
}
