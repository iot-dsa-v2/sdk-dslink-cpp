#include "dsa/network.h"
#include "dsa/stream.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"

#include "message/request/invoke_request_message.h"
#include "stream/responder/outgoing_invoke_stream.h"
#include "responder/invoke_node_model.h"

using namespace dsa;

namespace responder_invoke_test {
class MockNode : public InvokeNodeModel {
 public:
  ref_<OutgoingInvokeStream> last_invoke_stream;
  ref_<const InvokeRequestMessage> last_invoke_request;

  explicit MockNode(LinkStrandRef strand) : InvokeNodeModel(std::move(strand)){};

  void on_invoke(ref_<OutgoingInvokeStream> &&stream, ref_<NodeState> & parent) final {
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
class MockStreamAcceptor : public OutgoingStreamAcceptor {
 public:
  ref_<OutgoingInvokeStream> last_invoke_stream;
  ref_<const InvokeRequestMessage> last_invoke_request;

  void add(ref_<OutgoingInvokeStream> &&stream) override {
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

  void add(ref_<OutgoingSubscribeStream> &&stream) {}
  void add(ref_<OutgoingListStream> &&stream) override {}
  void add(ref_<OutgoingSetStream> &&stream) override {}
};
}

TEST(ResponderTest, Invoke_Model) {
  typedef responder_invoke_test::MockNode MockNode;
  App app;

  TestConfig server_config(app);

  MockNode *root_node = new MockNode(server_config.strand);

  server_config.strand->set_responder_model(ModelRef(root_node));

  typedef responder_invoke_test::MockStreamAcceptor MockStreamAcceptor;

  WrapperConfig client_config = server_config.get_client_config(app, true);

  //  auto tcp_server(new TcpServer(server_config));
  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  auto tcp_client = make_ref_<Client>(client_config);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  auto first_request = make_ref_<InvokeRequestMessage>();
  first_request->set_value(Var("hello"));

  auto second_request = make_ref_<InvokeRequestMessage>();
  second_request->set_value(Var("world"));

  ref_<const InvokeResponseMessage> last_response;

  auto invoke_stream = tcp_client->get_session().requester.invoke(
      "",
      [&](IncomingInvokeStream &stream,
          ref_<const InvokeResponseMessage> &&msg) {
        last_response = std::move(msg);
      },
      copy_ref_(first_request));

  // wait for acceptor to receive the request
  ASYNC_EXPECT_TRUE(500, *server_config.strand, [&]() -> bool {
    return root_node->last_invoke_request != nullptr;
  });
  // received request option should be same as the original one
  auto request_body = root_node->last_invoke_request->get_body();
  EXPECT_TRUE(request_body != nullptr && !request_body->empty());
  Var parsed_request_body =
      Var::from_msgpack(request_body->data(), request_body->size());
  EXPECT_TRUE(parsed_request_body.to_string() == "hello");

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() -> bool { return last_response != nullptr; });
  auto response_body = last_response->get_body();

  EXPECT_TRUE(response_body != nullptr && !response_body->empty());
  Var parsed_response_body =
      Var::from_msgpack(response_body->data(), response_body->size());
  EXPECT_TRUE(parsed_response_body.to_string() == "dsa");

  // close the invoke stream
  last_response.reset();
  root_node->last_invoke_stream->close();

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() -> bool { return last_response != nullptr; });
  EXPECT_EQ(last_response->get_status(), MessageStatus::CLOSED);

  ASYNC_EXPECT_TRUE(500, *server_config.strand, [&]() -> bool {
    return root_node->last_invoke_stream->is_destroyed() &&
           root_node->last_invoke_stream->ref_count() == 1;
  });

  ASYNC_EXPECT_TRUE(500, *client_config.strand, [&]() -> bool {
    return invoke_stream->is_destroyed() && invoke_stream->ref_count() == 1;
  });

  tcp_server->destroy_in_strand(tcp_server);
destroy_client_in_strand(tcp_client);

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}

TEST(ResponderTest, Invoke_Acceptor) {
  typedef responder_invoke_test::MockStreamAcceptor MockStreamAcceptor;
  App app;

  MockStreamAcceptor *mock_stream_acceptor = new MockStreamAcceptor();

  TestConfig server_config(app);
  server_config.strand->set_stream_acceptor(
      std::unique_ptr<MockStreamAcceptor>(mock_stream_acceptor));

  WrapperConfig client_config = server_config.get_client_config(app, true);

  //  auto tcp_server(new TcpServer(server_config));
  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  auto tcp_client = make_ref_<Client>(client_config);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  auto first_request = make_ref_<InvokeRequestMessage>();
  first_request->set_value(Var("hello"));

  auto second_request = make_ref_<InvokeRequestMessage>();
  second_request->set_value(Var("world"));

  ref_<const InvokeResponseMessage> last_response;
  auto invoke_stream = tcp_client->get_session().requester.invoke(
      "",
      [&](IncomingInvokeStream &stream,
          ref_<const InvokeResponseMessage> &&msg) {
        last_response = std::move(msg);
      },
      copy_ref_(first_request));

  // wait for acceptor to receive the request
  ASYNC_EXPECT_TRUE(500, *server_config.strand, [&]() -> bool {
    return mock_stream_acceptor->last_invoke_request != nullptr;
  });
  // received request option should be same as the original one
  auto request_body = mock_stream_acceptor->last_invoke_request->get_body();
  EXPECT_TRUE(request_body != nullptr && !request_body->empty());
  Var parsed_request_body =
      Var::from_msgpack(request_body->data(), request_body->size());
  EXPECT_TRUE(parsed_request_body.to_string() == "hello");

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() -> bool { return last_response != nullptr; });
  auto response_body = last_response->get_body();

  EXPECT_TRUE(response_body != nullptr && !response_body->empty());
  Var parsed_response_body =
      Var::from_msgpack(response_body->data(), response_body->size());
  EXPECT_TRUE(parsed_response_body.to_string() == "dsa");

  // close the invoke stream from the re
  invoke_stream->close();

  ASYNC_EXPECT_TRUE(500, *client_config.strand, [&]() -> bool {
    return invoke_stream->is_destroyed() && invoke_stream->ref_count() == 1;
  });

  ASYNC_EXPECT_TRUE(500, *server_config.strand, [&]() -> bool {
    return mock_stream_acceptor->last_invoke_request == nullptr;
  });

  tcp_server->destroy_in_strand(tcp_server);
destroy_client_in_strand(tcp_client);

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
