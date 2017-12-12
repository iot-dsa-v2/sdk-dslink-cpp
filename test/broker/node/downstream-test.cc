#include "dsa/message.h"
#include "dsa/responder.h"
#include "dsa/stream.h"
#include "dsa/util.h"

#include "../util/broker_runner.h"
#include "broker.h"
#include "config/broker_config.h"
#include "gtest/gtest.h"

#include "core/client.h"
#include "module/logger.h"

using namespace dsa;

namespace broker_downstream_test {

class MockNodeAction : public InvokeNodeModel {
 public:
  explicit MockNodeAction(LinkStrandRef strand)
      : InvokeNodeModel(std::move(strand)){};

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

class MockNodeValue : public NodeModel {
 public:
  explicit MockNodeValue(LinkStrandRef strand) : NodeModel(std::move(strand)) {
    set_value(Var("hello world"));
  };
};
class MockNodeRoot : public NodeModel {
 public:
  explicit MockNodeRoot(LinkStrandRef strand) : NodeModel(std::move(strand)) {
    add_list_child("value", make_ref_<MockNodeValue>(_strand));
    add_list_child("action", make_ref_<MockNodeAction>(_strand));
  };
};
}

TEST(BrokerDownstreamTest, Subscribe) {
  typedef broker_downstream_test::MockNodeRoot MockNodeRoot;

  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();

  WrapperStrand client_strand = get_client_wrapper_strand(broker);
  client_strand.strand->set_responder_model(
      ModelRef(new MockNodeRoot(client_strand.strand)));
  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect([&](const shared_ptr_<Connection>& connection) {
    tcp_client->get_session().requester.subscribe(
        "downstream/test/value",
        [&](IncomingSubscribeStream& stream,
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
  typedef broker_downstream_test::MockNodeRoot MockNodeRoot;

  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();

  WrapperStrand client_strand = get_client_wrapper_strand(broker);
  client_strand.strand->set_responder_model(
      ModelRef(new MockNodeRoot(client_strand.strand)));
  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect([&](const shared_ptr_<Connection>& connection) {

    ref_<InvokeRequestMessage> invoke_req = make_ref_<InvokeRequestMessage>();
    invoke_req->set_target_path("downstream/test/action");
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

TEST(BrokerDownstreamTest, List) {
  typedef broker_downstream_test::MockNodeRoot MockNodeRoot;

  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();

  WrapperStrand client_strand1 = get_client_wrapper_strand(broker, "test1");
  client_strand1.strand->set_responder_model(
      ModelRef(new MockNodeRoot(client_strand1.strand)));
  auto tcp_client1 = make_ref_<Client>(client_strand1);

  WrapperStrand client_strand2 = get_client_wrapper_strand(broker, "test2");
  auto tcp_client2 = make_ref_<Client>(client_strand2);

  // after client1 disconnected, list update should show it's disconnected
  auto test_disconnection_list = [&]() {
    tcp_client2->get_session().requester.list(
        "downstream/test1",
        [&](IncomingListStream&, ref_<const ListResponseMessage>&& msg) {
          EXPECT_EQ(msg->get_status(), MessageStatus::NOT_AVAILABLE);
          // end the test

          client_strand2.strand->post([tcp_client2, &client_strand2]() {
            tcp_client2->destroy();
            client_strand2.destroy();
          });

          broker->strand->post([broker]() { broker->destroy(); });
        });
  };

  // downstream should has test1 and test2 nodes
  auto test_downstream_list = [&]() {
    tcp_client2->get_session().requester.list(
        "downstream",
        [&](IncomingListStream&, ref_<const ListResponseMessage>&& msg) {
          auto map = msg->get_map();
          EXPECT_TRUE(map["test1"]->get_value().is_map());
          EXPECT_TRUE(map["test2"]->get_value().is_map());
          test_disconnection_list();
        });
  };

  // when list on downstream/test1 it should have a metadata for test1's dsid
  auto test_downstream_child_list =
      [&](const shared_ptr_<Connection>& connection) {
        tcp_client1->get_session().requester.list(
            "downstream/test1",
            [&](IncomingListStream&, ref_<const ListResponseMessage>&& msg) {
              auto map = msg->get_map();
              EXPECT_EQ(map["$$dsid"]->get_value().to_string(),
                        tcp_client1->get_session().dsid());

              client_strand1.strand->post([tcp_client1, &client_strand1]() {
                tcp_client1->destroy();
                client_strand1.destroy();
              });
              test_downstream_list();
            });

      };

  tcp_client1->connect(std::move(test_downstream_child_list));
  tcp_client2->connect();

  broker->run();
  EXPECT_TRUE(broker->is_destroyed());
}