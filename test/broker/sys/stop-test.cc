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

TEST(BrokerSysTest, StopBroker) {
  string_ token = "12345678901234567890123456789012";
  string_to_file(token, ".close_token");

  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();
  broker->run(false);
  EXPECT_TRUE(broker->get_active_server_port() != 0);

  WrapperStrand client_strand = get_client_wrapper_strand(broker);
  client_strand.strand->set_responder_model(
      ModelRef(new NodeModel(client_strand.strand)));
  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect([&](const shared_ptr_<Connection>& connection) {

    ref_<InvokeRequestMessage> invoke_req = make_ref_<InvokeRequestMessage>();
    invoke_req->set_target_path("sys/stop");
    invoke_req->set_value(Var(token));

    tcp_client->get_session().requester.invoke(
        [&](IncomingInvokeStream&, ref_<const InvokeResponseMessage>&& msg) {
          EXPECT_EQ(msg->get_status(), MessageStatus::CLOSED);
          client_strand.strand->post([&]() {
            tcp_client->destroy();
            client_strand.destroy();
          });

        },
        std::move(invoke_req));
  });
  broker->wait();
  EXPECT_TRUE(broker->is_destroyed());
}
