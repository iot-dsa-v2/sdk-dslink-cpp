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

using namespace dsa;

using BrokerSysTest = SetUpBase;

TEST_F(BrokerSysTest, TokenTest) {
  Storage::get_config_bucket().remove_all();

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
      get_client_wrapper_strand(broker, "Test1", protocol());
  auto client_1 = make_ref_<Client>(client_strand1);

  bool allow_all_set = false;
  string_ token;
  bool client_2_connected = false;
  int client_3_connected = 0;
  client_1->connect([&](const shared_ptr_<Connection> connection) {

    // send set request
    client_1->get_session().requester.set(
        CAST_LAMBDA(IncomingSetStreamCallback)[&](
            IncomingSetStream & stream, ref_<const SetResponseMessage> && msg) {
          EXPECT_TRUE(msg->get_status() == Status::DONE);
          allow_all_set = true;
        },
        make_ref_<SetRequestMessage>("Sys/Clients/Allow_All", Var(false)));

    client_1->get_session().requester.invoke(
        CAST_LAMBDA(IncomingInvokeStreamCallback)[&](
            IncomingInvokeStream & stream,
            ref_<const InvokeResponseMessage> && msg) {
          EXPECT_TRUE(msg->get_status() == Status::DONE);
          Var v = msg->get_value();
          EXPECT_TRUE(v.is_map());
          token = v["Token"].to_string();
          EXPECT_FALSE(token.empty());
        },
        make_ref_<InvokeRequestMessage>("Sys/Tokens/Add",
                                        Var({{"Count", Var(1)}})));

  });
  WAIT_EXPECT_TRUE(1000,
                   [&]() -> bool { return allow_all_set && !token.empty(); });

  // connect link 2 without token
  WrapperStrand client_strand2 =
      get_client_wrapper_strand(broker, "Test1", protocol());
  auto client_2 = make_ref_<Client>(client_strand2);
  client_2->connect([&](const shared_ptr_<Connection> connection) {
    client_2_connected = true;
  });

  // connect link 3 with token,
  WrapperStrand client_strand3 =
      get_client_wrapper_strand(broker, "Test1", protocol());
  client_strand3.client_token = token;
  auto client_3 = make_ref_<Client>(client_strand3);
  client_3->connect(
      [&](const shared_ptr_<Connection> connection) {
        EXPECT_FALSE(client_2_connected);
        client_3_connected++;
        if (client_3_connected == 1) {
          // disconnect it once and see if it still reconnect
          connection->destroy();
        }
      },
      Client::EVERY_CONNECTION);

  WAIT_EXPECT_TRUE(1000, [&]() -> bool {
    return client_3_connected == 2 && !client_2_connected;
  });

  client_strand1.strand->post([&]() {
    client_1->destroy();
    client_strand1.destroy();
  });
  client_strand2.strand->post([&]() {
    client_2->destroy();
    client_strand2.destroy();
  });
  client_strand3.strand->post([&]() {
    client_3->destroy();
    client_strand3.destroy();
  });
  broker->strand->post([&]() { broker->destroy(); });

  broker->wait();
  EXPECT_TRUE(broker->is_destroyed());
}