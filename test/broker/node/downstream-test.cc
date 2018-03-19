#include "dsa/message.h"
#include "dsa/responder.h"
#include "dsa/stream.h"
#include "dsa/util.h"

#include <gtest/gtest.h>
#include "../util/broker_runner.h"
#include "broker.h"
#include "config/broker_config.h"

#include "core/client.h"
#include "module/logger.h"

#include "../../sdk/test_config.h"
#include "dslink.h"

using namespace dsa;

using BrokerDownstreamTest = SetUpBase;

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
  explicit MockNodeValue(const LinkStrandRef &strand)
      : NodeModel(strand, PermissionLevel::WRITE) {
    set_value(Var("hello world"));
  };
};
class MockNodeRoot : public NodeModel {
 public:
  explicit MockNodeRoot(const LinkStrandRef &strand) : NodeModel(strand) {
    add_list_child("Value", make_ref_<MockNodeValue>(_strand));

    // add a child action
    auto node = make_ref_<NodeModel>(_strand->get_ref());
    add_list_child("Node", node->get_ref());
    node->add_list_child("Action", make_ref_<MockNodeAction>(_strand));
  };
};
}

TEST_F(BrokerDownstreamTest, Subscribe) {
  typedef broker_downstream_test::MockNodeRoot MockNodeRoot;

  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();
  broker->run(false);
  EXPECT_TRUE(broker->get_active_server_port() != 0);

  WrapperStrand client_strand =
      get_client_wrapper_strand(broker, "Test", protocol());
  client_strand.strand->set_responder_model(
      ModelRef(new MockNodeRoot(client_strand.strand)));
  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect([&](const shared_ptr_<Connection>& connection) {
    tcp_client->get_session().requester.subscribe(
        "Downstream/Test/Value",
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
  broker->wait();
  EXPECT_TRUE(broker->is_destroyed());
}

TEST_F(BrokerDownstreamTest, Invoke) {
  typedef broker_downstream_test::MockNodeRoot MockNodeRoot;

  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();
  broker->run(false);
  EXPECT_TRUE(broker->get_active_server_port() != 0);

  WrapperStrand client_strand =
      get_client_wrapper_strand(broker, "Test", protocol());
  client_strand.strand->set_responder_model(
      ModelRef(new MockNodeRoot(client_strand.strand)));
  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect([&](const shared_ptr_<Connection>& connection) {

    ref_<InvokeRequestMessage> invoke_req = make_ref_<InvokeRequestMessage>();
    invoke_req->set_target_path("Downstream/Test/Node/Action");
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
  broker->wait();
  EXPECT_TRUE(broker->is_destroyed());
}

TEST_F(BrokerDownstreamTest, Set) {
  typedef broker_downstream_test::MockNodeRoot MockNodeRoot;

  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();
  broker->run(false);
  EXPECT_TRUE(broker->get_active_server_port() != 0);

  WrapperStrand client_strand =
      get_client_wrapper_strand(broker, "Test", protocol());
  client_strand.strand->set_responder_model(
      ModelRef(new MockNodeRoot(client_strand.strand)));
  auto tcp_client = make_ref_<Client>(client_strand);

  int step = 0;

  tcp_client->connect([&](const shared_ptr_<Connection>& connection) {
    tcp_client->get_session().requester.subscribe(
        "Downstream/Test/Value",
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
                  make_ref_<SetRequestMessage>("Downstream/Test/Value",
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
  broker->wait();
  EXPECT_TRUE(broker->is_destroyed());
}

TEST_F(BrokerDownstreamTest, List) {
  typedef broker_downstream_test::MockNodeRoot MockNodeRoot;

  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();
  broker->run(false);
  EXPECT_TRUE(broker->get_active_server_port() != 0);

  WrapperStrand client_strand1 =
      get_client_wrapper_strand(broker, "Test1", protocol());
  client_strand1.strand->set_responder_model(
      ModelRef(new MockNodeRoot(client_strand1.strand)));
  auto tcp_client1 = make_ref_<Client>(client_strand1);

  WrapperStrand client_strand2 =
      get_client_wrapper_strand(broker, "Test2", protocol());
  auto tcp_client2 = make_ref_<Client>(client_strand2);

  // after client1 disconnected, list update should show it's disconnected
  auto step_3_disconnection_list = [&]() {
    tcp_client2->get_session().requester.list(
        "Downstream/Test1",
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
        "Downstream",
        [&](IncomingListStream&, ref_<const ListResponseMessage>&& msg) {
          auto map = msg->get_map();
          EXPECT_TRUE(map["Test1"]->get_value().is_map());
          EXPECT_TRUE(map["Test2"]->get_value().is_map());
          step_3_disconnection_list();
        });
  };

  // when list on downstream/test1 it should have a metadata for test1's dsid
  auto step_1_downstream_child_list =
      [&](const shared_ptr_<Connection>& connection) {
        tcp_client1->get_session().requester.list(
            "Downstream/Test1",
            [&](IncomingListStream&, ref_<const ListResponseMessage>&& msg) {
              auto map = msg->get_map();
              EXPECT_EQ(map["$$dsid"]->get_value().to_string(),
                        client_strand1.get_dsid());

              client_strand1.strand->post([tcp_client1, &client_strand1]() {
                tcp_client1->destroy();
                client_strand1.destroy();
              });
              step_2_downstream_list();
            });

      };

  tcp_client1->connect(std::move(step_1_downstream_child_list));
  tcp_client2->connect();

  broker->wait();
  EXPECT_TRUE(broker->is_destroyed());
}

TEST_F(BrokerDownstreamTest, ListDisconnect) {
  typedef broker_downstream_test::MockNodeRoot MockNodeRoot;

  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();
  broker->run(false);
  EXPECT_TRUE(broker->get_active_server_port() != 0);

  WrapperStrand client_strand1 =
      get_client_wrapper_strand(broker, "Test1", protocol());
  auto tcp_client1 = make_ref_<Client>(client_strand1);

  WrapperStrand client_strand2 =
      get_client_wrapper_strand(broker, "Test2", protocol());
  client_strand2.strand->set_responder_model(
      ModelRef(new MockNodeRoot(client_strand2.strand)));
  auto tcp_client2 = make_ref_<Client>(client_strand2);
  shared_ptr_<Connection> connection2;

  // list again after previous list is closed
  auto step_5 = [&]() {
    tcp_client1->get_session().requester.list(
        "Downstream/Test2",
        [&](IncomingListStream& stream, ref_<const ListResponseMessage>&& msg) {
          EXPECT_EQ(msg->get_status(), MessageStatus::OK);

          // end the test
          client_strand1.strand->post([tcp_client1, &client_strand1]() {
            tcp_client1->destroy();
            client_strand1.destroy();
          });
          client_strand2.strand->post([tcp_client2, &client_strand2]() {
            tcp_client2->destroy();
            client_strand2.destroy();
          });
          broker->strand->post([broker]() { broker->destroy(); });
        });

  };

  int step = 0;
  // when list on downstream/test1 it should have a metadata for test1's dsid
  auto step_1_to_4 = [&](const shared_ptr_<Connection>& connection) {
    tcp_client1->get_session().requester.list(
        "Downstream/Test2",
        [&](IncomingListStream& stream, ref_<const ListResponseMessage>&& msg) {
          step++;
          switch (step) {
            case 1: {
              // step 1, connect client 2
              EXPECT_EQ(msg->get_status(), MessageStatus::NOT_AVAILABLE);
              tcp_client2->connect([&](const shared_ptr_<Connection>& conn) {
                connection2 = conn;
              });
              break;
            }
            case 2: {
              // step 2, disconnect client 2
              EXPECT_EQ(msg->get_status(), MessageStatus::OK);
              connection2->destroy_in_strand(connection2);
              break;
            }

            case 3: {
              // step 3, disconnected
              EXPECT_EQ(msg->get_status(), MessageStatus::NOT_AVAILABLE);
              break;
            }

            case 4: {
              // step 4, reconnected
              EXPECT_EQ(msg->get_status(), MessageStatus::OK);

              stream.close();

              // list again
              client_strand1.strand->post(std::move(step_5));
              break;
            }
            default:
              throw "unexpected";
          }

        });
  };

  tcp_client1->connect(std::move(step_1_to_4));

  broker->wait();
  EXPECT_TRUE(broker->is_destroyed());
}

TEST_F(BrokerDownstreamTest, ListChildDisconnect) {
  typedef broker_downstream_test::MockNodeRoot MockNodeRoot;

  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();
  broker->run(false);
  EXPECT_TRUE(broker->get_active_server_port() != 0);
  WrapperStrand client_strand1 =
      get_client_wrapper_strand(broker, "Test1", protocol());
  auto tcp_client1 = make_ref_<Client>(client_strand1);

  WrapperStrand client_strand2 =
      get_client_wrapper_strand(broker, "Test2", protocol());
  client_strand2.strand->set_responder_model(
      ModelRef(new MockNodeRoot(client_strand2.strand)));
  auto tcp_client2 = make_ref_<Client>(client_strand2);
  shared_ptr_<Connection> connection2;

  // list again after previous list is closed
  auto step_5 = [&]() {
    tcp_client1->get_session().requester.list(
        "Downstream/Test2/Value",
        [&](IncomingListStream& stream, ref_<const ListResponseMessage>&& msg) {
          EXPECT_EQ(msg->get_status(), MessageStatus::OK);

          // end the test
          client_strand1.strand->post([tcp_client1, &client_strand1]() {
            tcp_client1->destroy();
            client_strand1.destroy();
          });
          client_strand2.strand->post([tcp_client2, &client_strand2]() {
            tcp_client2->destroy();
            client_strand2.destroy();
          });
          broker->strand->post([broker]() { broker->destroy(); });
        });

  };

  int step = 0;

  // when list on downstream/test1 it should have a metadata for test1's dsid
  auto step_1_to_4 = [&](const shared_ptr_<Connection>& connection) {
    tcp_client1->get_session().requester.list(
        "Downstream/Test2/Value",
        [&](IncomingListStream& stream, ref_<const ListResponseMessage>&& msg) {
          step++;
          switch (step) {
            case 1: {
              // step 1, connect client 2
              EXPECT_EQ(msg->get_status(), MessageStatus::NOT_AVAILABLE);
              tcp_client2->connect([&](const shared_ptr_<Connection>& conn) {
                connection2 = conn;
              });
              break;
            }
            case 2: {
              // step 2, disconnect client 2
              EXPECT_EQ(msg->get_status(), MessageStatus::OK);
              connection2->destroy_in_strand(connection2);
              break;
            }

            case 3: {
              // step 3, disconnected
              EXPECT_EQ(msg->get_status(), MessageStatus::NOT_AVAILABLE);
              break;
            }

            case 4: {
              // step 4, reconnected, close stream
              EXPECT_EQ(msg->get_status(), MessageStatus::OK);

              stream.close();

              // list again
              client_strand1.strand->post(step_5);
              break;
            }
            default:
              throw "unexpected";
          }

        });
  };

  tcp_client1->connect(step_1_to_4);
  broker->wait();
  EXPECT_TRUE(broker->is_destroyed());
}

TEST_F(BrokerDownstreamTest, ListChildBeforeParent) {
  typedef broker_downstream_test::MockNodeRoot MockNodeRoot;

  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();
  broker->run(false);
  EXPECT_TRUE(broker->get_active_server_port() != 0);

  WrapperStrand client_strand =
      get_client_wrapper_strand(broker, "Test1", protocol());
  client_strand.strand->set_responder_model(
      ModelRef(new MockNodeRoot(client_strand.strand)));
  auto tcp_client = make_ref_<Client>(client_strand);

  auto step_2_list_parent = [&]() {
    tcp_client->get_session().requester.list(
        "Downstream/Test1/Node",
        [&](IncomingListStream& stream, ref_<const ListResponseMessage>&& msg) {
          EXPECT_EQ(msg->get_status(), MessageStatus::OK);
          EXPECT_TRUE((*msg->get_parsed_map())["Action"].is_map());
          // end the test
          client_strand.strand->post([tcp_client, &client_strand]() {
            tcp_client->destroy();
            client_strand.destroy();
          });
          broker->strand->post([broker]() { broker->destroy(); });
        });

  };

  auto step_1_list_child = [&](const shared_ptr_<Connection>& connection) {
    tcp_client->get_session().requester.list(
        "Downstream/Test1/Node/Action",
        [&](IncomingListStream& stream, ref_<const ListResponseMessage>&& msg) {
          EXPECT_EQ(msg->get_status(), MessageStatus::OK);
          auto mapref = msg->get_parsed_map();
          EXPECT_EQ((*mapref)["$invokable"].to_string(), "write");
          client_strand.strand->post(std::move(step_2_list_parent));
        });

  };

  tcp_client->connect(std::move(step_1_list_child));

  broker->wait();
  EXPECT_TRUE(broker->is_destroyed());
}
