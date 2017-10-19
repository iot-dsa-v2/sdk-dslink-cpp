#include <module/logger.h>
#include "dsa/network.h"
#include "dsa/stream.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"

#include "message/request/set_request_message.h"
#include "stream/responder/outgoing_set_stream.h"

using namespace dsa;

namespace responder_set_test {
class MockNode : public NodeModelBase {
public:
  ref_<OutgoingSetStream> last_set_stream;
  ref_<const SetRequestMessage> last_set_request;

  explicit MockNode(LinkStrandRef strand) : NodeModelBase(std::move(strand)){};

  void on_set(ref_<OutgoingSetStream> &&stream) override {
    BOOST_ASSERT_MSG(last_set_stream == nullptr,
                     "receive second subscription stream, not expected");
    last_set_stream = stream;
    stream->on_request([this](OutgoingSetStream &s,
                              ref_<const SetRequestMessage> &&message) {
      last_set_request = std::move(message);
    });
    auto response = make_ref_<SetResponseMessage>();
    stream->send_response(std::move(response));
  }
};
class MockStreamAcceptor : public OutgoingStreamAcceptor {
public:
  ref_<OutgoingSetStream> last_set_stream;
  ref_<const SetRequestMessage> last_set_request;

  void add(ref_<OutgoingSetStream> &&stream) override {
    BOOST_ASSERT_MSG(last_set_stream == nullptr,
                     "receive second subscription stream, not expected");
    last_set_stream = stream;
    stream->on_request([this](OutgoingSetStream &s,
                              ref_<const SetRequestMessage> &&message) {
      last_set_request = std::move(message);
    });
    auto response = make_ref_<SetResponseMessage>();
    stream->send_response(std::move(response));
  }

  void add(ref_<OutgoingSubscribeStream> &&stream) {}
  void add(ref_<OutgoingListStream> &&stream) override {}
  void add(ref_<OutgoingInvokeStream> &&stream) override {}
};
}
//
//TEST(ResponderTest, Set_Model) {
//  typedef responder_set_test::MockNode MockNode;
//  App app;
//
//  TestConfig server_config(app);
//
//  MockNode *root_node = new MockNode(server_config.strand);
//
//  server_config.get_link_config()->set_responder_model(ModelRef(root_node));
//
//  typedef responder_set_test::MockStreamAcceptor MockStreamAcceptor;
//
//  WrapperConfig client_config = server_config.get_client_config(app, true);
//
//  //  auto tcp_server(new TcpServer(server_config));
//  auto tcp_server = make_shared_<TcpServer>(server_config);
//  tcp_server->start();
//
//  auto tcp_client = make_shared_<Client>(client_config);
//  tcp_client->connect();
//
//  ASYNC_EXPECT_TRUE(500, *client_config.strand,
//                    [&]() { return tcp_client->get_session().is_connected(); });
//
//  auto first_request = make_ref_<SetRequestMessage>();
//  first_request->set_value(Variant("hello"));
//
//  auto second_request = make_ref_<SetRequestMessage>();
//  second_request->set_value(Variant("world"));
//
//  ref_<const SetResponseMessage> last_response;
//  // test invalid path scenario
//  auto set_stream = tcp_client->get_session().requester.set(
//    "",
//    [&](IncomingSetStream &stream,
//        ref_<const SetResponseMessage> &&msg) {
//      last_response = std::move(msg);
//    },
//    copy_ref_(first_request));
//
//  // wait for acceptor to receive the request
//  ASYNC_EXPECT_TRUE(500, *server_config.strand, [&]() -> bool {
//    return root_node->last_set_request != nullptr;
//  });
//  // received request option should be same as the original one
//  auto request_body = root_node->last_set_request->get_body();
//  EXPECT_TRUE(request_body != nullptr && !request_body->empty());
//  Variant parsed_request_body =
//    Variant::from_msgpack(request_body->data(), request_body->size());
//  EXPECT_TRUE(parsed_request_body.is_string() &&
//              parsed_request_body.get_string() == "hello");
//
//  ASYNC_EXPECT_TRUE(500, *client_config.strand,
//                    [&]() -> bool { return last_response != nullptr; });
//
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
//}

TEST(ResponderTest, Set_Acceptor) {
  typedef responder_set_test::MockStreamAcceptor MockStreamAcceptor;
  App app;

  MockStreamAcceptor *mock_stream_acceptor = new MockStreamAcceptor();

  TestConfig server_config(app);
  server_config.get_link_config()->set_stream_acceptor(
    std::unique_ptr<MockStreamAcceptor>(mock_stream_acceptor));

  WrapperConfig client_config = server_config.get_client_config(app, true);
  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  auto tcp_client = make_shared_<Client>(client_config);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  auto first_request = make_ref_<SetRequestMessage>();
  first_request->set_value(Variant("hello"));

  auto second_request = make_ref_<SetRequestMessage>();
  second_request->set_value(Variant("world"));

  ref_<const SetResponseMessage> last_response;
  // test invalid path scenario
  auto set_stream = tcp_client->get_session().requester.set(
    "",
    [&](IncomingSetStream &stream,
        ref_<const SetResponseMessage> &&msg) {
      last_response = std::move(msg);
    },
    copy_ref_(first_request));

  // wait for acceptor to receive the request
  ASYNC_EXPECT_TRUE(500, *server_config.strand, [&]() -> bool {
    return mock_stream_acceptor->last_set_request != nullptr;
  });
  // received request option should be same as the original one
  MessageValue request_value = mock_stream_acceptor->last_set_request->get_value();
  EXPECT_TRUE(request_value.value.is_string() && request_value.value.get_string() == "hello");

  ASYNC_EXPECT_TRUE(5000000, *client_config.strand,
                    [&]() -> bool { return last_response != nullptr; });

  Server::close_in_strand(tcp_server);
  Client::close_in_strand(tcp_client);

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
