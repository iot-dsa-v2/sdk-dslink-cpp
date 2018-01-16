#include "dsa/message.h"
#include "dsa/network.h"
#include "dsa/responder.h"
#include "dsa/stream.h"

#include "../async_test.h"
#include "../test_config.h"
#include <gtest/gtest.h>

#include "core/client.h"
#include "network/tcp/tcp_server.h"

#include "message/request/invoke_request_message.h"
#include "module/logger.h"
#include "responder/invoke_node_model.h"
#include "stream/responder/outgoing_invoke_stream.h"

using namespace dsa;

static const int32_t big_str_size = 100000;

TEST(ResponderTest, Paged_Invoke_Request) {
  auto app = std::make_shared<App>();

  TestConfig server_strand(app);
  Var last_request;
  SimpleInvokeNode *root_node = new SimpleInvokeNode(
      server_strand.strand->get_ref(),
      [&](Var &&v, SimpleInvokeNode &node, OutgoingInvokeStream &stream) {
        last_request = std::move(v);
        stream.close();
      });

  server_strand.strand->set_responder_model(ModelRef(root_node));

  //  auto tcp_server(new TcpServer(server_strand));
  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();

  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  string_ big_str1;
  string_ big_str2;
  big_str1.resize(big_str_size);
  // big_str2.resize(big_str_size);
  for (int32_t i = 0; i < big_str_size; ++i) {
    big_str1[i] = static_cast<char>(i % 26 + 'a');
    // big_str2[i] = static_cast<char>((i + 13) % 26 + 'a');
  }
  auto first_request = make_ref_<InvokeRequestMessage>();
  first_request->set_value(Var(big_str1));

  //  auto second_request = make_ref_<InvokeRequestMessage>();
  //  second_request->set_value(Var(big_str2));

  ref_<const InvokeResponseMessage> last_response;

  auto invoke_stream = tcp_client->get_session().requester.invoke(
      [&](IncomingInvokeStream &stream,
          ref_<const InvokeResponseMessage> &&msg) {
        last_response = std::move(msg);
      },
      copy_ref_(first_request));

  // wait for acceptor to receive the request
  ASYNC_EXPECT_TRUE(1000, *server_strand.strand,
                    [&]() -> bool { return !last_request.is_null(); });
  // received request option should be same as the original one
  string_ s_result = last_request.to_string();

  EXPECT_TRUE(s_result == big_str1);

  // TODO : send second request

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() -> bool { return last_response != nullptr; });
  EXPECT_TRUE(last_response->get_status() == MessageStatus::CLOSED);

  // close the invoke stream
  last_response.reset();
  invoke_stream->close();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand, [&]() -> bool {
    return invoke_stream->is_destroyed() && invoke_stream->ref_count() == 1;
  });

  tcp_server->destroy_in_strand(tcp_server);
  destroy_client_in_strand(tcp_client);

  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  server_strand.destroy();
  client_strand.destroy();
  app->wait();
}

TEST(ResponderTest, Paged_Invoke_Response) {
  auto app = std::make_shared<App>();

  TestConfig server_strand(app);

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

  //  auto tcp_server(new TcpServer(server_strand));
  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();

  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  auto first_request = make_ref_<InvokeRequestMessage>();

  ref_<const InvokeResponseMessage> first_response;
  ref_<const InvokeResponseMessage> last_response;

  auto invoke_stream = tcp_client->get_session().requester.invoke(
      [&](IncomingInvokeStream &stream,
          ref_<const InvokeResponseMessage> &&msg) {
        // cache the first response and last response
        if (first_response == nullptr) {
          first_response = msg;
        }
        last_response = std::move(msg);
      },
      copy_ref_(first_request));

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand, [&]() -> bool {
    // check if last page is received
    return last_response != nullptr &&
           last_response->get_page_id() + 1 == -first_response->get_page_id();
  });

  string_ response_str1 = first_response->get_value().to_string();

  EXPECT_TRUE(response_str1 == big_str1);

  // TODO :check second response

  // close the invoke stream
  last_response.reset();
  invoke_stream->close();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand, [&]() -> bool {
    return invoke_stream->is_destroyed() && invoke_stream->ref_count() == 1;
  });

  tcp_server->destroy_in_strand(tcp_server);
  destroy_client_in_strand(tcp_client);

  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  server_strand.destroy();
  client_strand.destroy();
  app->wait();
}

TEST(ResponderTest, PagedSubscribeResponse) {
  auto app = std::make_shared<App>();

  TestConfig server_strand(app);

  string_ big_str1;
  big_str1.resize(big_str_size);
  for (int32_t i = 0; i < big_str_size; ++i) {
    big_str1[i] = static_cast<char>(i % 26 + 'a');
  }

  NodeModel *root_node =
      new NodeModel(server_strand.strand->get_ref(), PermissionLevel::WRITE);
  root_node->set_value(Var(big_str1));

  server_strand.strand->set_responder_model(ModelRef(root_node));

  //  auto tcp_server(new TcpServer(server_strand));
  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();

  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  ref_<const SubscribeResponseMessage> first_response;
  ref_<const SubscribeResponseMessage> last_response;

  auto subscribe_stream = tcp_client->get_session().requester.subscribe(
      "", [&](IncomingSubscribeStream &stream,
              ref_<const SubscribeResponseMessage> &&msg) {
        if (first_response == nullptr) {
          first_response = msg;
        }
        last_response = std::move(msg);
      });

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand, [&]() -> bool {
    // check if last page is received
    return last_response != nullptr &&
           last_response->get_page_id() + 1 == -first_response->get_page_id();
  });

  string_ response_str1 = first_response->get_value().value.to_string();

  EXPECT_TRUE(response_str1 == big_str1);

  // close the stream
  first_response.reset();
  last_response.reset();
  subscribe_stream->close();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand, [&]() -> bool {
    return subscribe_stream->is_destroyed() &&
           subscribe_stream->ref_count() == 1;
  });

  tcp_server->destroy_in_strand(tcp_server);
  destroy_client_in_strand(tcp_client);

  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  server_strand.destroy();
  client_strand.destroy();
  app->wait();
}

TEST(ResponderTest, PagedSetRequest) {
  auto app = std::make_shared<App>();

  TestConfig server_strand(app);

  string_ big_str1;
  big_str1.resize(big_str_size);
  for (int32_t i = 0; i < big_str_size; ++i) {
    big_str1[i] = static_cast<char>(i % 26 + 'a');
  }

  NodeModel *root_node =
      new NodeModel(server_strand.strand->get_ref(), PermissionLevel::WRITE);
  root_node->set_value(Var(big_str1));

  server_strand.strand->set_responder_model(ModelRef(root_node));

  //  auto tcp_server(new TcpServer(server_strand));
  auto tcp_server = server_strand.create_server();
  tcp_server->start();

  WrapperStrand client_strand = server_strand.get_client_wrapper_strand();

  auto tcp_client = make_ref_<Client>(client_strand);
  tcp_client->connect();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand,
                    [&]() { return tcp_client->get_session().is_connected(); });

  auto request = make_ref_<SetRequestMessage>();
  request->set_value(MessageValue(Var(big_str1)));

  ref_<const SetResponseMessage> last_response;

  auto set_stream = tcp_client->get_session().requester.set(
      [&](IncomingSetStream &stream, ref_<const SetResponseMessage> &&message) {
        last_response = std::move(message);
      },
      request);

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand, [&]() -> bool {
    return last_response != nullptr &&
           last_response->get_status() == MessageStatus::CLOSED;
  });

  string_ response_str1 = root_node->get_cached_value().value.to_string();

  EXPECT_TRUE(response_str1 == big_str1);

  // close the stream
  last_response.reset();

  ASYNC_EXPECT_TRUE(1000, *client_strand.strand, [&]() -> bool {
    return set_stream->is_destroyed() && set_stream->ref_count() == 1;
  });

  tcp_server->destroy_in_strand(tcp_server);
  destroy_client_in_strand(tcp_client);

  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }

  server_strand.destroy();
  client_strand.destroy();
  app->wait();
}
