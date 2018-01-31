#include "dsa/network.h"
#include "dsa/requester.h"
#include "dsa/responder.h"
#include "dsa/stream.h"

#include <math.h>
#include <atomic>
#include <boost/program_options.hpp>
#include <chrono>
#include <ctime>
#include <iostream>
#include "core/client.h"
#include "module/default/console_logger.h"
#include "module/default/simple_security.h"
#include "module/default/simple_session_manager.h"
#include "module/logger.h"
#include "network/tcp/tcp_server.h"
#include "network/ws/ws_callback.h"
#include "network/ws/ws_client_connection.h"
#include "util/date_time.h"

using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

namespace opts = boost::program_options;

using namespace dsa;

namespace broker_benchmark {

class BenchmarkNodeValue : public NodeModel {
 public:
  explicit BenchmarkNodeValue(LinkStrandRef strand)
      : NodeModel(std::move(strand)) {
    update_property("$type", Var("number"));
  };
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
                                        const string_& dsid_prefix,
                                        const string_& protocol) {
  WrapperStrand client_strand;
  client_strand.tcp_host = "127.0.0.1";
  client_strand.tcp_port = 4120;

  const uint8_t private_key[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  client_strand.strand = make_ref_<EditableStrand>(
      app->new_strand(), make_unique_<ECDH>(private_key, 32));

  client_strand.strand->set_session_manager(
      make_ref_<SimpleSessionManager>(client_strand.strand));

  client_strand.strand->set_client_manager(make_ref_<SimpleClientManager>());
  client_strand.strand->set_authorizer(
      make_ref_<SimpleAuthorizer>(client_strand.strand));

  auto logger = make_shared_<ConsoleLogger>();
  client_strand.strand->set_logger(std::move(logger));

  client_strand.strand->logger().level = Logger::ERROR_;

  if (!protocol.compare("ws")) {
    client_strand.ws_host = "127.0.0.1";
    client_strand.ws_port = 8080;
    client_strand.ws_path = "/";

    client_strand.client_connection_maker = [
      dsid_prefix = dsid_prefix, ws_host = client_strand.ws_host,
      ws_port = client_strand.ws_port
    ](LinkStrandRef & strand)->shared_ptr_<Connection> {
      return make_shared_<WsClientConnection>(strand, dsid_prefix, ws_host,
                                              ws_port);
    };
  } else {
    client_strand.client_connection_maker = [
      dsid_prefix = dsid_prefix, tcp_host = client_strand.tcp_host,
      tcp_port = client_strand.tcp_port
    ](LinkStrandRef & strand)->shared_ptr_<Connection> {
      return make_shared_<TcpClientConnection>(strand, dsid_prefix, tcp_host,
                                               tcp_port);
    };
  }
  return std::move(client_strand);
}

int main(int argc, const char* argv[]) {
  typedef broker_benchmark::BenchmarkNodeRoot BenchmarkNodeRoot;

  opts::options_description desc{"Options"};
  desc.add_options()("help,h", "Help screen")  //
      ("client,c", opts::value<int>()->default_value(2),
       "Number of Clients")  //
      ("point,p", opts::value<int>()->default_value(1000),
       "Number of Points per Client")  //
      ("protocol", opts::value<string_>()->default_value("ds"),
       "Protocol(ds/dss/ws/wss")("num-message,n",
                                 opts::value<int>()->default_value(10),
                                 "Message per second per Point")  //
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
  string_ protocol = variables["protocol"].as<string_>();
  int num_message = variables["num-message"].as<int>();
  Message::decode_all = variables["decode-value"].as<bool>();

  auto app = std::make_shared<App>(8);

  std::vector<shared_ptr_<Client>> clients;
  std::vector<ref_<EditableStrand>> strands;
  std::vector<ref_<BenchmarkNodeRoot>> root_nodes;
  std::vector<int64_t> message_receive_count;
  clients.reserve(client_count);
  message_receive_count.reserve(client_count);

  for (int i = 0; i < client_count; ++i) {
    message_receive_count.emplace_back(0);

    WrapperStrand strand = get_client_wrapper_strand(
        app, "benchmark" + std::to_string(i), protocol);
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

  int interval_ms = 5;
  int msg_per_interval = num_message * interval_ms / 1000;

  if (msg_per_interval == 0) {
    msg_per_interval = 1;
    interval_ms = 1000 / num_message;
  }
  boost::posix_time::milliseconds interval(interval_ms);
  boost::asio::deadline_timer timer(app->io_service(), interval);

  int64_t last_count = 0;
  int64_t last_time = DateTime::ms_since_epoch();
  std::function<void(const boost::system::error_code&)> timer_callback =
      [&](const boost::system::error_code& error) {
        try {
          int64_t current_time = DateTime::ms_since_epoch();
          int64_t count = 0;
          for (int i = 0; i < client_count; ++i) {
            count += message_receive_count[i];
          }
          if (current_time - last_time > 1000) {
            std::cout << std::endl
                      << "per second: " << ceil((count - last_count) * 1000.0 /
                                                (current_time - last_time))
                      << " total: " << count;
            last_time = current_time;
            last_count = count;
          }
          for (int i = 0; i < client_count; ++i) {
            strands[i]->dispatch([&, i]() {
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
