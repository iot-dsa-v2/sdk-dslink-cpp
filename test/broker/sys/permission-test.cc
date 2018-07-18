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
#include "responder/invoke_node_model.h"

using namespace dsa;

namespace broker_permission_test {

class MockNodeRoot : public NodeModel {
 public:
  explicit MockNodeRoot(const LinkStrandRef& strand) : NodeModel(strand) {
    add_list_child("action-read", make_ref_<SimpleInvokeNode>(
                                      strand, [](Var&& v) { return Var("ok"); },
                                      PermissionLevel::READ));
    add_list_child(
        "action-write",
        make_ref_<SimpleInvokeNode>(strand, [](Var&& v) { return Var("ok"); },
                                    PermissionLevel::WRITE));
    add_list_child(
        "action-config",
        make_ref_<SimpleInvokeNode>(strand, [](Var&& v) { return Var("ok"); },
                                    PermissionLevel::CONFIG));
  };
};
}  // namespace broker_permission_test

using BrokerSysTest = SetUpBase;

TEST_F(BrokerSysTest, PermissionTest) {
  Storage::get_config_bucket().remove_all();
  TestConfig::create_pem_files();

  // First Create Broker
  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();
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
      get_client_wrapper_strand(broker, "test1", protocol());
  client_strand1.strand->set_responder_model(ModelRef(
      new broker_permission_test::MockNodeRoot(client_strand1.strand)));

  auto client_1 = make_ref_<Client>(client_strand1);

  bool allow_all_set = false;
  string_ token;
  client_1->connect([&](const shared_ptr_<Connection> connection) {

    // send set request
    client_1->get_session().set(
        CAST_LAMBDA(IncomingSetStreamCallback)[&](
            IncomingSetStream & stream, ref_<const SetResponseMessage> && msg) {
          EXPECT_EQ(msg->get_status(), Status::DONE);
          allow_all_set = true;
        },
        make_ref_<SetRequestMessage>("sys/clients/allow-all", Var(false)));

    client_1->get_session().invoke(
        CAST_LAMBDA(IncomingInvokeStreamCallback)[&](
            IncomingInvokeStream & stream,
            ref_<const InvokeResponseMessage> && msg) {
          EXPECT_EQ(msg->get_status(), Status::DONE);
          Var v = msg->get_value();
          EXPECT_TRUE(v.is_map());
          token = v["Token"].to_string();
          EXPECT_FALSE(token.empty());
        },
        make_ref_<InvokeRequestMessage>(
            "sys/tokens/add", Var({{"Count", Var(1)}, {"Role", Var("G2")}})));
    client_1->get_session().invoke(
        CAST_LAMBDA(IncomingInvokeStreamCallback)[&](
            IncomingInvokeStream & stream,
            ref_<const InvokeResponseMessage> && msg) {
          EXPECT_EQ(msg->get_status(), Status::DONE);
        },
        make_ref_<InvokeRequestMessage>("sys/roles/add-role",
                                        Var({{"Name", Var("G2")}})));
    client_1->get_session().set(
        CAST_LAMBDA(IncomingSetStreamCallback)[&](
            IncomingSetStream & stream, ref_<const SetResponseMessage> && msg) {
          EXPECT_EQ(msg->get_status(), Status::DONE);
        },
        make_ref_<SetRequestMessage>("sys/roles/G2", Var("none")));

    client_1->get_session().invoke(
        CAST_LAMBDA(IncomingInvokeStreamCallback)[&](
            IncomingInvokeStream & stream,
            ref_<const InvokeResponseMessage> && msg) {
          EXPECT_EQ(msg->get_status(), Status::DONE);
        },
        make_ref_<InvokeRequestMessage>("sys/roles/G2/add-rule",
                                        Var({{"Path", Var("downstream/test1")},
                                             {"Permission", Var("write")}})));

  });
  WAIT_EXPECT_TRUE(1000,
                   [&]() -> bool { return allow_all_set && !token.empty(); });

  // connect link 2 with token
  bool client_2_connected = false;
  WrapperStrand client_strand2 =
      get_client_wrapper_strand(broker, "test2", protocol());
  client_strand2.client_token = token;
  auto client_2 = make_ref_<Client>(client_strand2);

  client_2->connect([&](const shared_ptr_<Connection> connection) {
    client_2_connected = true;
  });

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return client_2_connected; });

  bool action_read_done = false;
  bool action_write_done = false;
  bool action_config_done = false;
  bool list_valid_done = false;
  bool list_invalid_done = false;
  client_2->get_strand().post([&]() {
    client_2->get_session().invoke(
        CAST_LAMBDA(IncomingInvokeStreamCallback)[&](
            IncomingInvokeStream & stream,
            ref_<const InvokeResponseMessage> && msg) {
          EXPECT_EQ(msg->get_status(), Status::DONE);
          action_read_done = true;
        },
        make_ref_<InvokeRequestMessage>("downstream/test1/action-read", Var()));

    client_2->get_session().invoke(
        CAST_LAMBDA(IncomingInvokeStreamCallback)[&](
            IncomingInvokeStream & stream,
            ref_<const InvokeResponseMessage> && msg) {
          EXPECT_EQ(msg->get_status(), Status::DONE);
          action_write_done = true;
        },
        make_ref_<InvokeRequestMessage>("downstream/test1/action-write",
                                        Var()));

    client_2->get_session().invoke(
        CAST_LAMBDA(IncomingInvokeStreamCallback)[&](
            IncomingInvokeStream & stream,
            ref_<const InvokeResponseMessage> && msg) {
          EXPECT_EQ(msg->get_status(), Status::PERMISSION_DENIED);
          action_config_done = true;
        },
        make_ref_<InvokeRequestMessage>("downstream/test1/action-config",
                                        Var()));

    client_2->get_session().list(
        "downstream/test1",
        CAST_LAMBDA(IncomingListStreamCallback)[&](
            IncomingListStream&, ref_<const ListResponseMessage> && msg) {
          EXPECT_EQ(msg->get_status(), Status::OK);
          list_valid_done = true;
        });

    client_2->get_session().list(
        "downstream",
        CAST_LAMBDA(IncomingListStreamCallback)[&](
            IncomingListStream&, ref_<const ListResponseMessage> && msg) {
          EXPECT_EQ(msg->get_status(), Status::PERMISSION_DENIED);
          list_invalid_done = true;
        });
  });

  WAIT_EXPECT_TRUE(1000, [&]() -> bool {
    return action_read_done && action_write_done && action_config_done &&
           list_valid_done && list_invalid_done;
  });

  client_strand1.strand->post([&]() {
    client_1->destroy();
    client_strand1.destroy();
  });
  client_strand2.strand->post([&]() {
    client_2->destroy();
    client_strand2.destroy();
  });
  broker->strand->post([&]() { broker->destroy(); });

  broker->wait();
  EXPECT_TRUE(broker->is_destroyed());

  Storage::get_config_bucket().remove_all();
}
