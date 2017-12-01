
#include "../util/broker_runner.h"
#include "broker.h"
#include "config/broker_config.h"
#include "dsa/util.h"
#include "gtest/gtest.h"
#include "message/request/invoke_request_message.h"
#include "message/response/invoke_response_message.h"
#include "module/logger.h"
#include "responder/invoke_node_model.h"
#include "responder/node_model.h"
#include "stream/requester/incoming_subscribe_stream.h"
#include "stream/responder/outgoing_invoke_stream.h"
#include "stream/requester/incoming_invoke_stream.h"

using namespace dsa;

namespace broker_downstream_test {

class MockNodeRoot : public InvokeNodeModel {
 public:
  explicit MockNodeRoot(LinkStrandRef strand)
      : InvokeNodeModel(std::move(strand)) {
    set_value(Var("hello world"));
  };

  void on_invoke(ref_<OutgoingInvokeStream>&& stream,
                 ref_<NodeState>& parent) final {
    stream->on_request([this](OutgoingInvokeStream& s,
                              ref_<const InvokeRequestMessage>&& msg) {
      if (msg != nullptr) {
        auto response = make_ref_<InvokeResponseMessage>();
        response->set_value(Var((msg->get_value().to_string() + " world")));
        response->set_status(MessageStatus::CLOSED);
        s.send_response(std::move(response));
      } else {
        // nullptr message means stream closed
      }
    });
  }
};
}

TEST(BrokerDownstreamTest, Subscribe) {
  using MockNodeRoot = broker_downstream_test::MockNodeRoot;

  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();

  WrapperStrand client_strand = get_client_wrapper_strand(broker);
  client_strand.strand->set_responder_model(
      ModelRef(new MockNodeRoot(client_strand.strand)));
  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect([&](const shared_ptr_<Connection>& connection) {
    tcp_client->get_session().requester.subscribe(
        "downstream/test", [&](IncomingSubscribeStream& stream,
                               ref_<const SubscribeResponseMessage>&& msg) {
          EXPECT_EQ(msg->get_value().value.to_string(), "hello world");

          // end the test
          client_strand.strand->post([tcp_client, &client_strand]() {
            tcp_client->destroy();
            client_strand.destroy();
          });
          broker->strand->post([broker]() { broker->destroy(); });
        });

  });
  broker->run();
  EXPECT_TRUE(broker->is_destroyed());
}

TEST(BrokerDownstreamTest, Invoke) {
  using MockNodeRoot = broker_downstream_test::MockNodeRoot;

  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();

  WrapperStrand client_strand = get_client_wrapper_strand(broker);
  client_strand.strand->set_responder_model(
      ModelRef(new MockNodeRoot(client_strand.strand)));
  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect([&](const shared_ptr_<Connection>& connection) {

    ref_<InvokeRequestMessage> invoke_req =
        make_ref_<InvokeRequestMessage>();
    invoke_req->set_target_path("downstream/test");
    invoke_req->set_value(Var("hello"));

    tcp_client->get_session().requester.invoke(
        [&](IncomingInvokeStream&, ref_<const InvokeResponseMessage>&& msg) {
          EXPECT_EQ(msg->get_value().to_string(), "hello world");

          // end the test
          client_strand.strand->post([tcp_client, &client_strand]() {
            tcp_client->destroy();
            client_strand.destroy();
          });
          broker->strand->post([broker]() { broker->destroy(); });
        },
        std::move(invoke_req));

  });
  broker->run();
  EXPECT_TRUE(broker->is_destroyed());
}