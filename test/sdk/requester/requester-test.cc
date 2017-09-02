#include "dsa/message.h"
#include "dsa/network.h"
#include "gtest/gtest.h"

#include "network/tcp/tcp_client.h"
#include "network/tcp/tcp_server.h"

#include "module/default_modules.h"

using namespace dsa;

class MockStreamAcceptor : public OutgoingStreamAcceptor {
 public:
  ref_<OutgoingSubscribeStream> last_subscribe_stream;
  void add(ref_<OutgoingSubscribeStream> &stream) {
    last_subscribe_stream = stream;
  }
  void add(ref_<OutgoingListStream> &stream) {}
  void add(ref_<OutgoingInvokeStream> &stream) {}
  void add(ref_<OutgoingSetStream> &stream) {}
};

TEST(RequesterTest, Subscribe) {
  App app;

  // capture and log request
  auto mock_stream_acceptor = make_shared_<MockStreamAcceptor>();

  auto modules = make_ref_<DefaultModules>(app);
  modules->set_stream_acceptor(mock_stream_acceptor);

  WrapperConfig server_config;
  server_config.tcp_host = "127.0.0.1";
  server_config.tcp_port = 8092;
  server_config.strand = std::move(modules);
  server_config.strand->logger().level = Logger::WARNING;

  app.async_start(10);

  //  auto tcp_server(new TcpServer(server_config));
  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  WrapperConfig client_config;
  client_config.tcp_host = "127.0.0.1";
  client_config.tcp_port = 8092;
  client_config.strand = make_ref_<DefaultModules>(app);
  client_config.strand->logger().level = Logger::WARNING;

  auto tcp_client = make_shared_<TcpClient>(client_config);
  tcp_client->connect();

  std::atomic_bool connected{false};

  // wait till all client is connected
  while (!connected) {
    (*client_config.strand)()->dispatch(
        [&]() { connected = tcp_client->get_session().is_connected(); });
    app.sleep(50);
  }
  SubscribeOptions options;
  options.queue_time = 0x1234;
  options.queue_size = 0x5678;

  tcp_client->get_session().requester.subscribe(
      "/path", [](ref_<SubscribeResponseMessage> &&msg,
                  IncomingSubscribeStream &stream) {},  options);

  app.sleep(500);

  EXPECT_TRUE(mock_stream_acceptor->last_subscribe_stream != nullptr);
  EXPECT_TRUE(options == mock_stream_acceptor->last_subscribe_stream->options);

  Server::close_in_strand(tcp_server);
  Client::close_in_strand(tcp_client);

  app.close();
  for (int i = 0; i < 10; ++i) {
    app.sleep(50);
    if (app.is_stopped()) {
      break;
    }
  }

  EXPECT_TRUE(app.is_stopped());

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
