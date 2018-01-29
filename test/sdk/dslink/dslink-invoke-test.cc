#include "dsa/message.h"
#include "dsa/network.h"
#include "dsa/responder.h"
#include "dsa/stream.h"
#include <gtest/gtest.h>

#include "../async_test.h"
#include "../test_config.h"
#include "network/tcp/tcp_server.h"

using namespace dsa;

using DslinkTest = SetUpBase;

namespace link_subscribe_test {

static const int32_t big_str_size = 100000;

TEST_F(DslinkTest, PagedInvokeResponse) {
  auto app = std::make_shared<App>();

  TestConfig server_strand(app, false, protocol());

  string_ big_str1;
  big_str1.resize(big_str_size);
  for (int32_t i = 0; i < big_str_size; ++i) {
    big_str1[i] = static_cast<char>(i % 26 + 'a');
  }

  SimpleInvokeNode *root_node = new SimpleInvokeNode(
      server_strand.strand->get_ref(),
      [&](Var &&v, SimpleInvokeNode &node, OutgoingInvokeStream &stream) {
        auto first_response = make_ref_<InvokeResponseMessage>();
        first_response->set_status(MessageStatus::CLOSED);
        first_response->set_value(Var(big_str1));
        stream.send_response(std::move(first_response));
      });

  server_strand.strand->set_responder_model(ModelRef(root_node));

  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  auto web_server = server_strand.create_webserver();
  web_server->start();

  // Create link
  auto link = server_strand.create_dslink();

  // connection
  bool is_connected = false;
  link->connect(
      [&](const shared_ptr_<Connection> connection) { is_connected = true; });

  ASYNC_EXPECT_TRUE(500, *link->strand, [&]() { return is_connected; });

  auto first_request = make_ref_<InvokeRequestMessage>();

  ref_<const InvokeResponseMessage> response;

  auto invoke_stream = link->invoke(
      [&](IncomingInvokeStream &stream,
          ref_<const InvokeResponseMessage> &&msg) {
        response = std::move(msg);
      },
      copy_ref_(first_request));

  ASYNC_EXPECT_TRUE(500, *link->strand,
                    [&]() -> bool { return response != nullptr; });

  string_ response_str1 = response->get_value().to_string();

  EXPECT_TRUE(response_str1 == big_str1);

  // close the invoke stream
  response.reset();
  invoke_stream->close();

  ASYNC_EXPECT_TRUE(500, *link->strand, [&]() -> bool {
    return invoke_stream->is_destroyed() && invoke_stream->ref_count() == 1;
  });

  tcp_server->destroy_in_strand(tcp_server);
  web_server->destroy();
  destroy_dslink_in_strand(link);

  server_strand.destroy();
  app->close();

  WAIT_EXPECT_TRUE(500, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();
}
}
