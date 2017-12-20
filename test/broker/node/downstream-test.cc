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

#include "../../sdk/async_test.h"
#include "dslink.h"
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

    // add a child action
    auto node = make_ref_<NodeModel>(_strand->get_ref());
    add_list_child("node", node->get_ref());
    node->add_list_child("action", make_ref_<MockNodeAction>(_strand));
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
    invoke_req->set_target_path("downstream/test/node/action");
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
  auto step_3_disconnection_list = [&]() {
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
  auto step_2_downstream_list = [&]() {
    tcp_client2->get_session().requester.list(
        "downstream",
        [&](IncomingListStream&, ref_<const ListResponseMessage>&& msg) {
          auto map = msg->get_map();
          EXPECT_TRUE(map["test1"]->get_value().is_map());
          EXPECT_TRUE(map["test2"]->get_value().is_map());
          step_3_disconnection_list();
        });
  };

  // when list on downstream/test1 it should have a metadata for test1's dsid
  auto step_1_downstream_child_list =
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
              step_2_downstream_list();
            });

      };

  tcp_client1->connect(std::move(step_1_downstream_child_list));
  tcp_client2->connect();

  broker->run();
  EXPECT_TRUE(broker->is_destroyed());
}



ref_<DsLink> create_dslink(std::shared_ptr<App> app, int port, string_ dslink_name) {
  std::string address =
      std::string("127.0.0.1:") + std::to_string(port);

  const char *argv[] = {"./test", "-b", address.c_str()};
  int argc = 3;
  auto link = make_ref_<DsLink>(argc, argv, dslink_name, "1.0.0", app);
  link->init_responder();

  return link;
}

TEST(BrokerDownstreamTest, BROKER_NOTAVAILABLE) {
  typedef std::vector<std::vector<string_>> ListResponses;

  auto app = make_shared_<App>();

  auto broker = create_broker(app);
  broker->run();

  bool is_connected_1 = false;
  auto link_1 = create_dslink(app, broker->tcp_server_port, "mydslink");
  link_1->connect([&](const shared_ptr_<Connection> connection) { is_connected_1 = true; });
  ASYNC_EXPECT_TRUE(2000, *link_1->strand, [&]() { return is_connected_1; });

  bool is_connected_2 = false;
  auto link_2 = create_dslink(app, broker->tcp_server_port, "mydslink_new");
  link_2->connect([&](const shared_ptr_<Connection> connection) { is_connected_2 = true; });
  ASYNC_EXPECT_TRUE(2000, *link_2->strand, [&]() { return is_connected_2; });

  // list on root node
  ListResponses root_list_responses;
  VarMap map;
  link_2->list("downstream/mydslink",
                 [&](IncomingListCache &cache, const std::vector<string_> &str) {
                   root_list_responses.push_back(str);
                   map = cache.get_map();
                 });

  ASYNC_EXPECT_TRUE(500, *link_1.get()->strand, [&]() { return root_list_responses.size() != 0; });

  link_1->strand->post([link_1]() {link_1->destroy();});
  WAIT(1000);
  EXPECT_TRUE(link_1->is_destroyed());

  auto status = MessageStatus::PERMISSION_DENIED;

  link_2->list("downstream/mydslink",
                 [&](IncomingListCache &cache, const std::vector<string_> &str) {
                   status = cache.get_status();
                 });

  ASYNC_EXPECT_TRUE(1000, *link_2.get()->strand, [&]() { return status == MessageStatus::NOT_AVAILABLE; });

  link_2->strand->post([link_2]() {link_2->destroy();});
  broker->strand->post([broker]() {broker->destroy();});

  app->close();
  WAIT_EXPECT_TRUE(500, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();
}