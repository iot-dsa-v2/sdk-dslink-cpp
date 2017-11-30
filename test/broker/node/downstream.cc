
#include "../util/broker_runner.h"
#include "broker.h"
#include "dsa/util.h"
#include "gtest/gtest.h"
#include "config/broker_config.h"

using namespace dsa;

TEST(BrokerNode, Downstream) {
  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();

  WrapperStrand client_strand = get_client_wrapper_strand(broker);

  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect(
      [&](const shared_ptr_<Connection>& connection) {
       app->close();
        broker->strand->post([broker](){
          broker->destroy();
        });
        client_strand.strand->post([tcp_client, &client_strand](){
          tcp_client->destroy();
          client_strand.destroy();
        });
      });
  broker->run();
  EXPECT_TRUE(broker->is_destroyed());
}
