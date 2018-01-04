#include "dsa/network.h"
#include "dsa/responder.h"
#include "dsa/stream.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"

#include "message/request/invoke_request_message.h"
#include "module/logger.h"
#include "responder/invoke_node_model.h"
#include "stream/responder/outgoing_invoke_stream.h"

using namespace dsa;

TEST(ResponderTest, Paged_Invoke_Request) {
  return;
  auto app = std::make_shared<App>();

  TestConfig server_strand(app);
  server_strand.strand->logger().level = Logger::ALL___;
  Var last_request;
  SimpleInvokeNode *root_node = new SimpleInvokeNode(
      server_strand.strand,
      [&, link = std::move(link) ](Var && v, SimpleInvokeNode & node,
                                   OutgoingInvokeStream & stream) {
        last_request = std::move(v);
      });

  server_strand.strand->set_responder_model(ModelRef(root_node));

  //  auto tcp_server(new TcpServer(server_strand));
  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();

  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(500, *client_strand.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  const int32_t big_str_size = 100000;
  string_ big_str1;
  string_ big_str2;
  big_str1.resize(big_str_size);
  big_str2.resize(big_str_size);
  for (int32_t i = 0; i < big_str_size; ++i) {
    big_str1[i] = static_cast<char>(i % 26 + 'a');
    big_str1[i] = static_cast<char>((i + 13) % 26 + 'a');
  }
  auto first_request = make_ref_<InvokeRequestMessage>();
  first_request->set_value(Var(big_str1));

  auto second_request = make_ref_<InvokeRequestMessage>();
  second_request->set_value(Var(big_str2));

  ref_<const InvokeResponseMessage> last_response;

  auto invoke_stream = tcp_client->get_session().requester.invoke(
      [&](IncomingInvokeStream &stream,
          ref_<const InvokeResponseMessage> &&msg) {
        last_response = std::move(msg);
      },
      copy_ref_(first_request));

  // wait for acceptor to receive the request
  ASYNC_EXPECT_TRUE(500, *server_strand.strand,
                    [&]() -> bool { return !last_request.is_null(); });
  // received request option should be same as the original one
  EXPECT_TRUE(last_request.to_string() == big_str1);

  ASYNC_EXPECT_TRUE(500, *client_strand.strand,
                    [&]() -> bool { return last_response != nullptr; });
  EXPECT_TRUE(last_response->get_value().to_string() == "dsa");

  // close the invoke stream
  last_response.reset();
  invoke_stream->close();

  ASYNC_EXPECT_TRUE(500, *client_strand.strand, [&]() -> bool {
    return invoke_stream->is_destroyed() && invoke_stream->ref_count() == 1;
  });

  tcp_server->destroy_in_strand(tcp_server);
  destroy_client_in_strand(tcp_client);

  app->close();

  WAIT_EXPECT_TRUE(500, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  server_strand.destroy();
  client_strand.destroy();
  app->wait();
}
