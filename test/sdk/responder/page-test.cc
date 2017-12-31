#include "dsa/network.h"
#include "dsa/stream.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"

#include "message/request/invoke_request_message.h"
#include "responder/invoke_node_model.h"
#include "stream/responder/outgoing_invoke_stream.h"

using namespace dsa;

namespace responder_invoke_test {
class MockNode : public InvokeNodeModel {
 public:
  ref_<OutgoingInvokeStream> last_invoke_stream;
  ref_<const InvokeRequestMessage> last_invoke_request;

  explicit MockNode(LinkStrandRef strand)
      : InvokeNodeModel(std::move(strand)){};

  void on_invoke(ref_<OutgoingInvokeStream> &&stream,
                 ref_<NodeState> &parent) final {
    BOOST_ASSERT_MSG(last_invoke_stream == nullptr,
                     "receive second invoke stream, not expected");
    last_invoke_stream = stream;
    stream->on_request([this](OutgoingInvokeStream &s,
                              ref_<const InvokeRequestMessage> &&message) {
      last_invoke_request = std::move(message);
    });
    auto response = make_ref_<InvokeResponseMessage>();
    response->set_value(Var("dsa"));
    stream->send_response(std::move(response));
  }
};
}

TEST(ResponderTest, Paged_Invoke_Response) {
  typedef responder_invoke_test::MockNode MockNode;
  auto app = std::make_shared<App>();

  TestConfig server_strand(app);

  MockNode *root_node = new MockNode(server_strand.strand);

  server_strand.strand->set_responder_model(ModelRef(root_node));

  //  auto tcp_server(new TcpServer(server_strand));
  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();

  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(500, *client_strand.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  auto first_request = make_ref_<InvokeRequestMessage>();
  first_request->set_value(Var("hello"));

  auto second_request = make_ref_<InvokeRequestMessage>();
  second_request->set_value(Var("world"));

  ref_<const InvokeResponseMessage> last_response;

  auto invoke_stream = tcp_client->get_session().requester.invoke(
      [&](IncomingInvokeStream &stream,
          ref_<const InvokeResponseMessage> &&msg) {
        last_response = std::move(msg);
      },
      copy_ref_(first_request));

  // wait for acceptor to receive the request
  ASYNC_EXPECT_TRUE(500, *server_strand.strand, [&]() -> bool {
    return root_node->last_invoke_request != nullptr;
  });
  // received request option should be same as the original one
  EXPECT_TRUE(root_node->last_invoke_request->get_value().to_string() ==
              "hello");

  ASYNC_EXPECT_TRUE(500, *client_strand.strand,
                    [&]() -> bool { return last_response != nullptr; });
  EXPECT_TRUE(last_response->get_value().to_string() == "dsa");

  // close the invoke stream
  last_response.reset();
  root_node->last_invoke_stream->close();

  ASYNC_EXPECT_TRUE(500, *client_strand.strand,
                    [&]() -> bool { return last_response != nullptr; });
  EXPECT_EQ(last_response->get_status(), MessageStatus::CLOSED);

  ASYNC_EXPECT_TRUE(500, *server_strand.strand, [&]() -> bool {
    return root_node->last_invoke_stream->is_destroyed() &&
           root_node->last_invoke_stream->ref_count() == 1;
  });

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
