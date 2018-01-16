#include "dsa/message.h"
#include "dsa/responder.h"
#include "dsa/stream.h"
#include "dsa/util.h"

#include "../util/broker_runner.h"
#include "broker.h"
#include "config/broker_config.h"
#include <gtest/gtest.h>

#include "core/client.h"
#include "module/logger.h"

#include "../../sdk/async_test.h"
#include "dslink.h"

using namespace dsa;

static const int32_t big_str_size = 100000;

namespace broker_page_test {

class MockNodeRoot : public NodeModel {
 public:
  static Var last_request;
  explicit MockNodeRoot(LinkStrandRef strand) : NodeModel(std::move(strand)) {
    add_list_child("action",
                   make_ref_<SimpleInvokeNode>(
                       _strand->get_ref(), [&](Var&& v) {
                         last_request = std::move(v);
                         return Var();
                       }));
  };
};
Var MockNodeRoot::last_request;
}

TEST(BrokerPageTest, Invoke_Request) {
  typedef broker_page_test::MockNodeRoot MockNodeRoot;

  string_ big_str1;
  big_str1.resize(big_str_size);
  for (int32_t i = 0; i < big_str_size; ++i) {
    big_str1[i] = static_cast<char>(i % 26 + 'a');
  }


  auto broker = create_broker();
  shared_ptr_<App>& app = broker->get_app();
  broker->run(false);
  EXPECT_TRUE(broker->get_active_server_port() != 0);

  WrapperStrand client_strand = get_client_wrapper_strand(broker);
  client_strand.strand->set_responder_model(
      ModelRef(new MockNodeRoot(client_strand.strand)));
  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect([&](const shared_ptr_<Connection>& connection) {

    ref_<InvokeRequestMessage> invoke_req = make_ref_<InvokeRequestMessage>();
    invoke_req->set_target_path("downstream/test/action");
    invoke_req->set_value(Var(big_str1));

    tcp_client->get_session().requester.invoke(
        [&](IncomingInvokeStream&, ref_<const InvokeResponseMessage>&& msg) {
          string_ request_str = MockNodeRoot::last_request.to_string();

          EXPECT_TRUE(request_str == big_str1);

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
