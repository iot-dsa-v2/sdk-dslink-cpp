#include "dsa_common.h"

#include "responder.h"

#include <functional>

#include "message/message_factory.h"
#include "message/request/invoke_request_message.h"
#include "message/request/list_request_message.h"
#include "message/request/set_request_message.h"
#include "message/request/subscribe_request_message.h"
#include "network/client.h"

namespace dsa {

Responder::Responder(const std::shared_ptr<App> &app, Config config)
    : GracefullyClosable(app), _state_manager(*app), _config(config) {}

void Responder::start() {
  // register with app instance
  register_this();

  // initialize new connection
  ClientPtr connection = _initialize_connection();

  // connect to broker
  connection->connect();
}

ClientPtr Responder::_initialize_connection() {
  auto on_connect = [this](const std::shared_ptr<Session> session) {
    _session = session;
    _session->start();
  };

  Connection::Config connection_config(_config.broker_hostname,
                                       _config.broker_port, on_connect);

  ClientPtr connection(_app->new_client(_config.protocol, connection_config));
  connection->set_message_handler(
      std::bind(&Responder::_message_handler, share_this<Responder>(),
                std::placeholders::_1, std::placeholders::_2));

  return std::move(connection);
}

void Responder::_message_handler(const std::shared_ptr<Session> &session,
                                 SharedBuffer buf) {
  std::unique_ptr<Message> message(parse_message(buf));
  switch (message->type()) {
    case SubscribeRequest:on_subscribe_request(dynamic_cast<SubscribeRequestMessage &>(*message));
      break;
    case InvokeRequest:on_invoke_request(dynamic_cast<InvokeRequestMessage &>(*message));
      break;
    case SetRequest:on_set_request(dynamic_cast<SetRequestMessage &>(*message));
      break;
    case ListRequest:on_list_request(dynamic_cast<ListRequestMessage &>(*message));
      break;
    default:return;
  }
}

void Responder::on_subscribe_request(SubscribeRequestMessage &message) {
  auto stream = std::make_shared<SubscribeMessageStream>(_session,
                                                         message.get_subscribe_options(),
                                                         _stream_count++,
                                                         message.request_id());
//  auto node_state = _state_manager.get_or_create();
//  _session->add_outgoing_subscription(stream);
}

void Responder::on_invoke_request(InvokeRequestMessage &message) {
  // TODO: implement this
}

void Responder::on_set_request(SetRequestMessage &message) {
  // TODO: implement this
}

void Responder::on_list_request(ListRequestMessage &message) {
  // TODO: implement this
}

}
