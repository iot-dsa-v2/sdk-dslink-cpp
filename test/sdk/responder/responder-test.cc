#include "dsa/message.h"
#include "dsa/network.h"

#include "../async_test.h"
#include "gtest/gtest.h"

#include "network/tcp/tcp_client.h"
#include "network/tcp/tcp_server.h"

using namespace dsa;

TEST(ResponderTest, Subscribe) {
  //  App app;
  //
  //  auto modules = make_ref_<DefaultModules>(app);
  //  MockStreamAcceptor *mock_stream_acceptor = new MockStreamAcceptor();
  //  modules->set_stream_acceptor(
  //      std::unique_ptr<MockStreamAcceptor>(mock_stream_acceptor));
  //
  //  WrapperConfig server_config;
  //  server_config.tcp_host = "127.0.0.1";
  //  server_config.tcp_port = 8090;
  //  server_config.strand = std::move(modules);
  //  server_config.strand->logger().level = Logger::WARN;
  //
  //  app.async_start(10);
  //
  //  //  auto tcp_server(new TcpServer(server_config));
  //  auto tcp_server = make_shared_<TcpServer>(server_config);
  //  tcp_server->start();
  //
  //  WrapperConfig client_config;
  //  client_config.tcp_host = "127.0.0.1";
  //  client_config.tcp_port = 8090;
  //  client_config.strand = make_ref_<DefaultModules>(app);
  //  client_config.strand->logger().level = Logger::WARN;
  //
  //  auto tcp_client = make_shared_<TcpClient>(client_config);
  //  tcp_client->connect();
  //
  //  ASYNC_EXPECT_TRUE(500, (*client_config.strand)(),
  //                    [&]() { return tcp_client->get_session().is_connected();
  //                    });
  //
  //  SubscribeOptions initial_options;
  //  initial_options.queue_time = 0x1234;
  //  initial_options.queue_size = 0x5678;
  //
  //  SubscribeOptions update_options;
  //  update_options.queue_time = 0x9876;
  //  update_options.queue_size = 0x5432;
  //
  //  ref_<const SubscribeResponseMessage> last_response;
  //  auto subscribe_stream = tcp_client->get_session().requester.subscribe(
  //      "/path",
  //      [&](ref_<const SubscribeResponseMessage> &&msg,
  //          IncomingSubscribeStream &stream) {
  //        last_response = std::move(msg);  // store response
  //      },
  //      initial_options);
  //
  //  // wait for acceptor to receive the request
  //  WAIT_EXPECT_TRUE(500, [&]() -> bool {
  //    return mock_stream_acceptor->last_subscribe_stream != nullptr;
  //  });
  //  // received request option should be same as the original one
  //  EXPECT_TRUE(initial_options ==
  //              mock_stream_acceptor->last_subscribe_stream->options());
  //
  //  WAIT_EXPECT_TRUE(500, [&]() -> bool { return last_response != nullptr; });
  //
  //  EXPECT_TRUE(last_response->get_value().has_value() &&
  //              last_response->get_value().value.is_string() &&
  //              last_response->get_value().value.get_string() == "hello");
  //
  //  // send an new request to udpate the option of the same stream
  //  auto second_request = make_ref_<SubscribeRequestMessage>();
  //  second_request->set_subscribe_option(update_options);
  //  subscribe_stream->send_message(
  //      std::move(second_request));  // send update options
  //
  //  WAIT_EXPECT_TRUE(500, [&]() -> bool {
  //    return mock_stream_acceptor->last_subscribe_options != nullptr;
  //  });
  //  // request option should be same as the second one
  //  EXPECT_TRUE(update_options ==
  //  *mock_stream_acceptor->last_subscribe_options);
  //  EXPECT_TRUE(update_options ==
  //              mock_stream_acceptor->last_subscribe_stream->options());
  //
  //  Server::close_in_strand(tcp_server);
  //  Client::close_in_strand(tcp_client);
  //
  //  app.close();
  //
  //  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });
  //
  //  if (!app.is_stopped()) {
  //    app.force_stop();
  //  }
  //
  //  app.wait();
}
