#include "dsa/message.h"
#include "dsa/network.h"

#include "../test/sdk/async_test.h"
#include "../test/sdk/test_config.h"

#include "network/tcp/tcp_client.h"
#include "network/tcp/tcp_server.h"

#include <chrono>
#include <ctime>

#include <atomic>

using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

using namespace dsa;


const int CLIENT_COUNT = 2;

class MockNode : public NodeModel {
public:
  explicit MockNode(LinkStrandRef strand) : NodeModel(std::move(strand)){};

};

int main() {
  App app;

  app.async_start(CLIENT_COUNT * 2 + 2);

  TestConfig server_config(app);

  MockNode *root_node = new MockNode(server_config.strand);

  server_config.get_link_config()->set_stream_acceptor(
    make_unique_<NodeStateManager>(ref_<MockNode>(root_node)));

  //  auto tcp_server(new TcpServer(server_config));
  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();


  std::vector<WrapperConfig> client_configs;
  std::vector<shared_ptr_ <TcpClient>> clients;
  std::atomic_int receive_count[CLIENT_COUNT];

  SubscribeOptions initial_options;
  initial_options.qos = QosLevel::_1;
  initial_options.queue_size = 655360;

  for (int i = 0; i < CLIENT_COUNT; ++i) {
    client_configs.emplace_back(server_config.get_client_config(app));
    clients.emplace_back(make_shared_<TcpClient>(client_configs[i]));
    clients[i]->connect();

    wait_for_bool(500, (*client_configs[i].strand)(),
                  [&]() { return clients[i]->get_session().is_connected(); });

    receive_count[i] = 0;

   std::atomic_int & count = receive_count[i];

    clients[i]->get_session().requester.subscribe(
      "",
      [&](ref_<const SubscribeResponseMessage> &&msg,
          IncomingSubscribeStream &stream) {
        count.fetch_add(1);
      },
      initial_options);
  }



  int msg_per_second = 100000;

  boost::posix_time::milliseconds interval(10);
  boost::asio::deadline_timer timer(app.io_service(), interval);

  int print_count = 0; // print every 100 timer visit;

  auto ts = high_resolution_clock::now();

  std::function<void(const boost::system::error_code&)> tick;
  tick = (*server_config.strand)()->wrap([&](const boost::system::error_code& error){

    if (!error) {
      auto ts2 = high_resolution_clock::now();

      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(ts2 - ts).count();
      ts = ts2;

      int count = 0;
      for (int i = 0; i < CLIENT_COUNT; ++i) {
        count += receive_count[i];
        receive_count[i] = 0;
      }
      count /= CLIENT_COUNT;

      print_count+=ms;
     if (print_count > 2000) {
        print_count = 0;
        std::cout << std::endl << "message per second: " << (msg_per_second * CLIENT_COUNT)<< "  current: " << count*1000 / ms  << " x " << CLIENT_COUNT << ", interval " << ms;
     }

      msg_per_second = (count * 1000/ms + msg_per_second * 9) / 10;
      if (msg_per_second < 1000) msg_per_second = 1000;
      int num_message = msg_per_second * ms / 800;
      for (int i = 0; i < num_message; ++i) {
        root_node->set_value(Variant(i));
      }

      timer.async_wait(tick);
    }
  });

  timer.async_wait(tick);




  std::cout << std::endl << "start benchmark for 60 seconds";
  sleep(60);
  std::cout << std::endl << "end benchmark";
  timer.cancel();


  Server::close_in_strand(tcp_server);
  for (int i = 0; i < CLIENT_COUNT; ++i) {
    Client::close_in_strand(clients[i]);
  }


  app.close();

  wait_for_bool(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
  return 0;
}
