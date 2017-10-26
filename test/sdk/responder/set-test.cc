
#include "dsa/network.h"
#include "dsa/responder.h"
#include "dsa/stream.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"
#include "module/logger.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"

#include "message/request/set_request_message.h"
#include "message/response/list_response_message.h"
#include "stream/responder/outgoing_set_stream.h"

using namespace dsa;

namespace responder_set_test {

/// define a node for the responder
class MockNode : public NodeModel {
 public:
  explicit MockNode(LinkStrandRef strand) : NodeModel(std::move(strand)){};

  bool allows_set_value() override { return true; }

  MessageStatus on_set_attribute(const std::string &field,
                                 Var &&value) override {
    update_property(field, std::move(value));
    return MessageStatus::CLOSED;
  }
};
class MockStreamAcceptor : public OutgoingStreamAcceptor {
 public:
  ref_<OutgoingSetStream> last_set_stream;
  ref_<const SetRequestMessage> last_set_request;

  void add(ref_<OutgoingSetStream> &&stream) override {
    BOOST_ASSERT_MSG(last_set_stream == nullptr,
                     "receive second set stream, not expected");
    last_set_stream = stream;
    stream->on_request(
        [this](OutgoingSetStream &s, ref_<const SetRequestMessage> &&message) {
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

TEST(ResponderTest, Set_Model) {
  typedef responder_set_test::MockNode MockNode;
  App app;
  // get the configs for unit testing
  TestConfig server_config(app);

  MockNode *root_node = new MockNode(server_config.strand);

  server_config.get_link_config()->set_responder_model(ModelRef(root_node));

  WrapperConfig client_config = server_config.get_client_config(app, true);

  auto tcp_server = make_shared_<TcpServer>(server_config);
  tcp_server->start();

  auto tcp_client = make_shared_<Client>(client_config);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  // subscribe on root node value
  ref_<const SubscribeResponseMessage> last_subscribe_response;
  auto subscribe_stream = tcp_client->get_session().requester.subscribe(
      "", [&](IncomingSubscribeStream &stream,
              ref_<const SubscribeResponseMessage> &&msg) {
        last_subscribe_response = std::move(msg);  // store response
      });

  // list on root node
  ref_<const ListResponseMessage> last_list_response;
  tcp_client->get_session().requester.list(
      "",
      [&](IncomingListStream &stream, ref_<const ListResponseMessage> &&msg) {
        last_list_response = msg;
      });

  // set request to change value
  auto first_request = make_ref_<SetRequestMessage>();
  first_request->set_value(Var("hello"));

  // set request to change attribute
  auto second_request = make_ref_<SetRequestMessage>();
  second_request->set_attribute_field("@attr");
  second_request->set_value(Var("world"));

  // send set request
  auto set_stream1 = tcp_client->get_session().requester.set(
      "",
      [&](IncomingSetStream &stream, ref_<const SetResponseMessage> &&msg) {},
      std::move(first_request));
  auto set_stream2 = tcp_client->get_session().requester.set(
      "",
      [&](IncomingSetStream &stream, ref_<const SetResponseMessage> &&msg) {},
      std::move(second_request));

  // wait until response of subscribe and list are received
  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() -> bool { return last_list_response != nullptr; });
  ASYNC_EXPECT_TRUE(500, *client_config.strand, [&]() -> bool {
    return last_subscribe_response != nullptr;
  });

  // check the subsciption response is same as the value set
  auto value = last_subscribe_response->get_value().value;
  EXPECT_TRUE(value.to_string() == "hello");

  // check the list response is same as the value set
  auto list_map = last_list_response->get_parsed_map();

  EXPECT_TRUE(list_map != nullptr && (*list_map)["@attr"].to_string() == "world");

  Server::destroy_in_strand(tcp_server);
  Client::destroy_in_strand(tcp_client);

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}

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
  first_request->set_value(Var("hello"));

  auto second_request = make_ref_<SetRequestMessage>();
  second_request->set_value(Var("world"));

  ref_<const SetResponseMessage> last_response;
  // test invalid path scenario
  auto set_stream = tcp_client->get_session().requester.set(
      "",
      [&](IncomingSetStream &stream, ref_<const SetResponseMessage> &&msg) {
        last_response = std::move(msg);
      },
      copy_ref_(first_request));

  // wait for acceptor to receive the request
  ASYNC_EXPECT_TRUE(500, *server_config.strand, [&]() -> bool {
    return mock_stream_acceptor->last_set_request != nullptr;
  });
  // received request option should be same as the original one
  MessageValue request_value =
      mock_stream_acceptor->last_set_request->get_value();
  EXPECT_TRUE(request_value.value.is_string() &&
              request_value.value.get_string() == "hello");

  ASYNC_EXPECT_TRUE(500, *client_config.strand,
                    [&]() -> bool { return last_response != nullptr; });

  Server::destroy_in_strand(tcp_server);
  Client::destroy_in_strand(tcp_client);

  app.close();

  WAIT_EXPECT_TRUE(500, [&]() { return app.is_stopped(); });

  if (!app.is_stopped()) {
    app.force_stop();
  }

  app.wait();
}
