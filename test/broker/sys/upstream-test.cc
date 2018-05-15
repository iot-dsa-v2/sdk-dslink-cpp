#include "dsa/message.h"
#include "dsa/responder.h"
#include "dsa/stream.h"
#include "dsa/util.h"

#include <gtest/gtest.h>
#include "../../sdk/async_test.h"
#include "../../sdk/test_config.h"
#include "../util/broker_runner.h"
#include "broker.h"
#include "config/broker_config.h"
#include "core/client.h"
#include "module/logger.h"
#include "responder/value_node_model.h"

using namespace dsa;

using BrokerSysTest = SetUpBase;

TEST_F(BrokerSysTest, UpstreamTest) {
  Storage::get_config_bucket().remove_all();

  // First Create Broker
  auto broker = create_broker();
  shared_ptr_<App> &app = broker->get_app();
  broker->run(false);
  ASYNC_EXPECT_TRUE(1000, *broker->strand,
                    [&]() { return broker->get_active_server_port() != 0; });
  int32_t port;

  switch (protocol()) {
    case dsa::ProtocolType::PROT_DSS:
      port = broker->get_active_secure_port();
      break;
    default:
      port = broker->get_active_server_port();
  }

  EXPECT_TRUE(port != 0);

  WrapperStrand client_strand1 =
      get_client_wrapper_strand(broker, "Test1", protocol());

  auto root_node = make_ref_<NodeModel>(client_strand1.strand);
  auto child_node =
      make_ref_<ValueNodeModel>(client_strand1.strand, "string",
                                [](const Var &) { return StatusDetail(); });
  child_node->set_value(Var("hello"));
  root_node->add_list_child("Value", std::move(child_node));
  client_strand1.strand->set_responder_model(ModelRef(root_node.get()));
  auto client_1 = make_ref_<Client>(client_strand1);

  bool upstream_added = false;
  client_1->connect([&](const shared_ptr_<Connection> connection) {

    client_1->get_session().invoke(
        CAST_LAMBDA(IncomingInvokeStreamCallback)[&](
            IncomingInvokeStream & stream,
            ref_<const InvokeResponseMessage> && msg) {
          EXPECT_TRUE(msg->get_status() == Status::DONE);
          upstream_added = true;
        },
        make_ref_<InvokeRequestMessage>(
            "Sys/Upstream/Add",
            Var({{"Node_Name", Var("up1")},
                 {"Connection_Name", Var("down1")},
                 {"Url", Var(string_("127.0.0.1:") + std::to_string(port))}})));

  });
  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return upstream_added; });

  string_ upstream_status = "";
  client_strand1.strand->dispatch([&]() {
    client_1->get_session().subscribe(
        "Sys/Upstream/up1/Status",
        CAST_LAMBDA(IncomingSubscribeStreamCallback)[&](
            IncomingSubscribeStream &,
            ref_<const SubscribeResponseMessage> && msg) {
          EXPECT_TRUE(msg->get_status() == Status::OK);
          upstream_status = msg->get_value().value.to_string();
        });
  });

  WAIT_EXPECT_TRUE(1000,
                   [&]() -> bool { return upstream_status == "Connected"; });

  bool value_upstream_checked = false;
  client_strand1.strand->dispatch([&]() {
    client_1->get_session().subscribe(
        "Upstream/up1/Downstream/Test1/Value",
        CAST_LAMBDA(IncomingSubscribeStreamCallback)[&](
            IncomingSubscribeStream &,
            ref_<const SubscribeResponseMessage> && msg) {
          EXPECT_TRUE(msg->get_status() == Status::OK);
          EXPECT_EQ(msg->get_value().value.to_string(), "hello");
          value_upstream_checked = true;
        });
  });
  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return value_upstream_checked; });

  bool value_downstream_checked = false;
  client_strand1.strand->dispatch([&]() {
    client_1->get_session().subscribe(
        "Downstream/down1/Downstream/Test1/Value",
        CAST_LAMBDA(IncomingSubscribeStreamCallback)[&](
            IncomingSubscribeStream &,
            ref_<const SubscribeResponseMessage> && msg) {
          EXPECT_TRUE(msg->get_status() == Status::OK);
          EXPECT_EQ(msg->get_value().value.to_string(), "hello");
          value_downstream_checked = true;
        });
  });
  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return value_downstream_checked; });

  client_strand1.strand->post([&]() {
    client_1->destroy();
    client_strand1.destroy();
  });

  broker->strand->post([&]() { broker->destroy(); });

  broker->wait();
  EXPECT_TRUE(broker->is_destroyed());
}