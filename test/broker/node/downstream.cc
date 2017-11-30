
#include "../util/broker_runner.h"
#include "broker.h"
#include "config/broker_config.h"
#include "dsa/util.h"
#include "gtest/gtest.h"
#include "module/logger.h"
#include "responder/node_model.h"
#include "stream/requester/incoming_subscribe_stream.h"

using namespace dsa;

namespace broker_downstream_test {

class MockNodeRoot : public NodeModel {
 public:
  explicit MockNodeRoot(LinkStrandRef strand) : NodeModel(std::move(strand)) {
    set_value(Var("hello world"));
  };
};
}

TEST(BrokerNode, Downstream) {
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
