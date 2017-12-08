#include "dsa/network.h"
#include "dsa/requester.h"
#include "dsa/responder.h"
#include "dsa/stream.h"

#include <chrono>
#include <ctime>
#include "core/client.h"
#include "module/logger.h"
#include "network/tcp/tcp_server.h"
#include "util/date_time.h"

#include <atomic>

#include <boost/program_options.hpp>

using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

namespace opts = boost::program_options;

using namespace dsa;

namespace broker_benchmark {

class BenchmarkNodeValue : public NodeModel {
public:
  explicit BenchmarkNodeValue(LinkStrandRef strand)
    : NodeModel(std::move(strand)){};
};
class BenchmarkNodeRoot : public NodeModel {
public:
  explicit BenchmarkNodeRoot(LinkStrandRef strand, int num_point)
    : NodeModel(std::move(strand)) {
    for (int i = 0; i < num_point; ++i) {
      add_list_child("v" + std::to_string(i),
                     make_ref_<BenchmarkNodeValue>(_strand));
    }
  };
  int64_t sub_value = 0;
  void new_value() {
    sub_value++;
    auto msg = make_ref_<SubscribeResponseMessage>();
    msg->set_value(MessageValue(Var(sub_value), DateTime::get_ts()));
    for (auto it : _list_children) {
      it.second->set_subscribe_response(msg->get_ref());
    }
  }
};
}

WrapperStrand get_client_wrapper_strand(shared_ptr_<App>& app,
                                        const string_& dsid_prefix) {
  WrapperStrand client_strand;
  client_strand.tcp_host = "127.0.0.1";
  client_strand.tcp_port = 4120;

  client_strand.strand = EditableStrand::make_default(app);
  client_strand.client_connection_maker =
    [
      dsid_prefix = dsid_prefix, tcp_host = client_strand.tcp_host,
      tcp_port = client_strand.tcp_port
    ](LinkStrandRef & strand, const string_& previous_session_id,
      int32_t last_ack_id)
      ->shared_ptr_<Connection> {
      return make_shared_<TcpClientConnection>(strand, dsid_prefix, tcp_host,
                                               tcp_port);
    };
  return std::move(client_strand);
}

int main(int argc, const char* argv[]) {
  typedef broker_benchmark::BenchmarkNodeRoot BenchmarkNodeRoot;

  opts::options_description desc{"Options"};
  desc.add_options()("help,h", "Help screen")  //
    ("client,c", opts::value<int>()->default_value(2),
     "Number of Clients")  //
    ("point,p", opts::value<int>()->default_value(2),
     "Number of Points per Client")  //
    ("num-message,n", opts::value<int>()->default_value(100),
     "Message per second per Point")  //
    ("encode-value,e", opts::bool_switch(),
     "Encode value before sending")  //
    ("decode-value,d", opts::bool_switch(),
     "Decode value after receiving")  //
    ;

  opts::variables_map variables;
  opts::store(opts::parse_command_line(argc, argv, desc), variables);
  opts::notify(variables);

  if (variables.count("help")) {
    std::cout << desc << '\n';
    return 0;
  }

  int client_count = variables["client"].as<int>();
  int point_count = variables["point"].as<int>();
  int num_message = variables["num-message"].as<int>();
  bool encode_value = variables["encode-value"].as<bool>();
  bool decode_value = variables["decode-value"].as<bool>();

  auto app = std::make_shared<App>();

  std::vector<shared_ptr_<Client>> clients;
  std::vector<ref_<EditableStrand>> strands;
  std::vector<ref_<BenchmarkNodeRoot>> root_nodes;
  std::vector<int64_t> message_receive_count;
  clients.reserve(client_count);
  message_receive_count.reserve(client_count);

  for (int i = 0; i < client_count; ++i) {
    message_receive_count.emplace_back(0);

    WrapperStrand strand =
      get_client_wrapper_strand(app, "benchmark" + std::to_string(i));
    auto client = make_shared_<Client>(strand);
    auto root_node = make_ref_<BenchmarkNodeRoot>(strand.strand, point_count);
    root_nodes.emplace_back(root_node);
    strand.strand->set_responder_model(std::move(root_node));
    clients.emplace_back(client);

    client->connect([
                      =, &count = message_receive_count[i], &client = clients[i]
                    ](const shared_ptr_<Connection>&) {

      SubscribeOptions options;
      options.qos = QosLevel::_1;
      for (int a = 0; a < client_count; ++a) {
        string_ node_path = "downstream/benchmark" + std::to_string(a);
        for (int b = 0; b < point_count; ++b) {
          string_ point_path = node_path + "/v" + std::to_string(b);
          client->get_session().requester.subscribe(
            point_path,
            [&count](IncomingSubscribeStream&,
                     ref_<const SubscribeResponseMessage>&&) { ++count; },
            options);
        }
      }
    });

    strands.emplace_back(strand.strand);
  }

  int interval_ms = 20;
  int msg_per_interval = num_message * interval_ms / 1000;

  if (msg_per_interval == 0) {
    msg_per_interval = 1;
    interval_ms = 1000 / num_message;
  }
  boost::posix_time::milliseconds interval(interval_ms);
  boost::asio::deadline_timer timer(app->io_service(), interval);

  

  int64_t last_count = 0;
  int64_t last_time = DateTime::time_since_epoch();
  std::function<void(const boost::system::error_code&)> timer_callback =
    [&](const boost::system::error_code& error) {
      try {
        int64_t current_time = DateTime::time_since_epoch();
        int64_t count = 0;
        for (int i = 0; i < client_count; ++i) {
          count += message_receive_count[i];
        }
        if (current_time - last_time > 1000) {
          std::cout << std::endl
                    << "per second: "
                    << (count - last_count) * 1000 / (current_time - last_time)
                    << " total: " << count;
          last_time = current_time;
          last_count = count;
        }
        for (int i = 0; i < client_count; ++i) {
          strands[i]->dispatch([&,i]() {
            auto& node = root_nodes[i];
            for (int j = 0; j < msg_per_interval; ++j) {
              node->new_value();
            }
          });
        }

      } catch (std::exception& e) {
        std::cout << std::endl << e.what();
      }
      timer.expires_from_now(interval);
      timer.async_wait(timer_callback);
    };
  timer.async_wait(timer_callback);
  app->wait();
}