#include "dsa/message.h"
#include "dsa/responder.h"
#include "dsa/stream.h"
#include "dsa/util.h"

#include <gtest/gtest.h>
#include "../../sdk/async_test.h"
#include "../util/broker_runner.h"
#include "broker.h"
#include "config/broker_config.h"
#include "core/client.h"
#include "dslink.h"
#include "module/logger.h"
#include "util/string.h"

using namespace dsa;

using BrokerSysTest = SetUpBase;

TEST_F(BrokerSysTest, StopBroker) {

  string_ bucket_name("config");
  string_ token = generate_random_string(48);

  SimpleSafeStorageBucket storage_bucket(bucket_name, nullptr, "");
  string_to_storage(token, default_master_token_path, storage_bucket);

  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();
    broker->run(false);
    ASYNC_EXPECT_TRUE(1000, *broker->strand,
                      [&]() { return broker->get_active_server_port() != 0; });

  WrapperStrand client_strand =
      get_client_wrapper_strand(broker, "test", protocol());
  client_strand.strand->set_responder_model(
      ModelRef(new NodeModel(client_strand.strand)));
  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect([&](const shared_ptr_<Connection>& connection) {

    ref_<InvokeRequestMessage> invoke_req = make_ref_<InvokeRequestMessage>();
    invoke_req->set_target_path("sys/stop");
    invoke_req->set_value(Var(token));

    tcp_client->get_session().invoke(
        [&](IncomingInvokeStream&, ref_<const InvokeResponseMessage>&& msg) {
          EXPECT_EQ(msg->get_status(), Status::DONE);
          client_strand.strand->post([&]() {
            tcp_client->destroy();
            client_strand.destroy();
          });

        },
        std::move(invoke_req));
  });
  broker->wait();
  storage_bucket.remove(default_master_token_path);
  EXPECT_TRUE(broker->is_destroyed());
}
