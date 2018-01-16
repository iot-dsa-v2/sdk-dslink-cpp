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
  explicit MockNodeValue(LinkStrandRef strand)
      : NodeModel(std::move(strand), PermissionLevel::WRITE) {
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

  auto app = std::make_shared<App>();
  auto broker = create_broker(app);
  broker->run();
  WAIT_EXPECT_TRUE(500,
                   [&]() { return broker->get_active_server_port() != 0; });

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
  app->close();
  WAIT_EXPECT_TRUE(500, [&]() -> bool { return app->is_stopped(); });
  if (!app->is_stopped()) { app->force_stop(); }
  app->wait();
  broker->destroy();
  EXPECT_TRUE(broker->is_destroyed());
}

TEST(BrokerDownstreamTest, Invoke) {
  typedef broker_downstream_test::MockNodeRoot MockNodeRoot;

  auto app = std::make_shared<App>();
  auto broker = create_broker(app);
  broker->run();
  WAIT_EXPECT_TRUE(500,
                   [&]() { return broker->get_active_server_port() != 0; });

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
  app->close();
  WAIT_EXPECT_TRUE(500, [&]() -> bool { return app->is_stopped(); });
  if (!app->is_stopped()) { app->force_stop(); }
  app->wait();
  broker->destroy();
  EXPECT_TRUE(broker->is_destroyed());
}

TEST(BrokerDownstreamTest, Set) {
  typedef broker_downstream_test::MockNodeRoot MockNodeRoot;

  auto app = std::make_shared<App>();
  auto broker = create_broker(app);
  broker->run();
  WAIT_EXPECT_TRUE(500,
                   [&]() { return broker->get_active_server_port() != 0; });

  WrapperStrand client_strand = get_client_wrapper_strand(broker);
  client_strand.strand->set_responder_model(
      ModelRef(new MockNodeRoot(client_strand.strand)));
  auto tcp_client = make_ref_<Client>(client_strand);

  int step = 0;

  tcp_client->connect([&](const shared_ptr_<Connection>& connection) {
    tcp_client->get_session().requester.subscribe(
        "downstream/test/value",
        [&](IncomingSubscribeStream& stream,
            ref_<const SubscribeResponseMessage>&& msg) {
          ++step;
          switch (step) {
            case 1: {
              EXPECT_EQ(msg->get_value().value.to_string(), "hello world");
              tcp_client->get_session().requester.set(
                  [](IncomingSetStream&,
                     ref_<const SetResponseMessage>&& set_response) {

                  },
                  make_ref_<SetRequestMessage>("downstream/test/value",
                                               Var("dsa")));
              break;
            }
            default: {
              EXPECT_EQ(msg->get_value().value.to_string(), "dsa");

              // end the test
              client_strand.strand->post([tcp_client, &client_strand]() {
                tcp_client->destroy();
                client_strand.destroy();
              });
              broker->strand->post([broker]() { broker->destroy(); });
            }
          }
        });

  });
  app->close();
  WAIT_EXPECT_TRUE(500, [&]() -> bool { return app->is_stopped(); });
  if (!app->is_stopped()) { app->force_stop(); }
  app->wait();
  broker->destroy();
  EXPECT_TRUE(broker->is_destroyed());
}

TEST(BrokerDownstreamTest, List) {
  typedef broker_downstream_test::MockNodeRoot MockNodeRoot;

  auto app = std::make_shared<App>();
  auto broker = create_broker(app);
  broker->run();
  WAIT_EXPECT_TRUE(500,
                   [&]() { return broker->get_active_server_port() != 0; });

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

  app->close();
  WAIT_EXPECT_TRUE(500, [&]() -> bool { return app->is_stopped(); });
  if (!app->is_stopped()) { app->force_stop(); }
  app->wait();
  broker->destroy();
  EXPECT_TRUE(broker->is_destroyed());
}

TEST(BrokerDownstreamTest, DownstreamNotAvailable) {
  typedef broker_downstream_test::MockNodeRoot MockNodeRoot;

  auto app = std::make_shared<App>();
  auto broker = create_broker(app);
  broker->run();
  WAIT_EXPECT_TRUE(2000,
                   [&]() { return broker->get_active_server_port() != 0; });

  WrapperStrand client_strand1 = get_client_wrapper_strand(broker, "test1");
  auto tcp_client1 = make_ref_<Client>(client_strand1);

  WrapperStrand client_strand2 = get_client_wrapper_strand(broker, "test2");
  client_strand2.strand->set_responder_model(
      ModelRef(new MockNodeRoot(client_strand2.strand)));
  auto tcp_client2 = make_ref_<Client>(client_strand2);

  // after client1 disconnected, list update should show it's disconnected
  auto step_2_list_on_disconnected_link = [&]() {
    tcp_client1->get_session().requester.list(
        "downstream/test2",
        [&](IncomingListStream& stream, ref_<const ListResponseMessage>&& msg) {
          EXPECT_EQ(msg->get_status(), MessageStatus::NOT_AVAILABLE);

          // end the test
          client_strand2.strand->post([tcp_client2, &client_strand2]() {
            tcp_client2->destroy();
            client_strand2.destroy();
          });

          broker->strand->post([broker]() { broker->destroy(); });
        });
  };

  int step = 0;
  // when list on downstream/test1 it should have a metadata for test1's dsid
  auto unavailable_child_list = [&](const shared_ptr_<Connection>& connection) {
    tcp_client1->get_session().requester.list(
        "downstream/test2",
        [&](IncomingListStream& stream, ref_<const ListResponseMessage>&& msg) {
          step++;
          switch (step) {
            case 1: {
              // step 1, connect client 2
              EXPECT_EQ(msg->get_status(), MessageStatus::NOT_AVAILABLE);
              tcp_client2->connect();
              break;
            }
            case 2: {
              // step 2, disconnect client 2
              EXPECT_EQ(msg->get_status(), MessageStatus::OK);
              client_strand2.strand->post([tcp_client2, &client_strand2]() {
                tcp_client2->destroy();
                client_strand2.destroy();
              });
              break;
            }

            default: {  //   case 3:{
              // step 3, end test
              EXPECT_EQ(msg->get_status(), MessageStatus::NOT_AVAILABLE);
              client_strand1.strand->post([tcp_client1, &client_strand1]() {
                tcp_client1->destroy();
                client_strand1.destroy();
              });
              broker->strand->post([broker]() { broker->destroy(); });
            }
          }

        });
  };

  tcp_client1->connect(std::move(unavailable_child_list));


  app->close();
  WAIT_EXPECT_TRUE(500, [&]() -> bool { return app->is_stopped(); });
  if (!app->is_stopped()) { app->force_stop(); }
  app->wait();
  broker->destroy();
  EXPECT_TRUE(broker->is_destroyed());
}
