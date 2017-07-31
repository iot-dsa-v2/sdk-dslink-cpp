#include "responder.h"

#include <functional>

#include "network/client.h"
#include "message/message_factory.h"
#include "message/request/subscribe_request_message.h"
#include "message/request/invoke_request_message.h"
#include "message/request/set_request_message.h"
#include "message/request/list_request_message.h"

namespace dsa {

Responder::Responder(const std::shared_ptr<App> &app, Config config)
    : GracefullyClosable(app),
      _state_manager(*app),
      _connection(app->new_client(
          config.protocol,
          Connection::Config(config.broker_hostname, config.broker_port))) {}

void Responder::start() {
  // register with app instance
  register_this();

  // set new message handler
  _connection->set_message_handler(
      std::bind(&Responder::_message_handler, share_this<Responder>(), 
                std::placeholders::_1, std::placeholders::_2));

  // connect to broker
  _connection->connect();
}

void Responder::_message_handler(const std::shared_ptr<Session> &session, SharedBuffer buf) {
  std::unique_ptr<Message> message(parse_message(buf));
  switch (message->type()) {
    case SubscribeRequest:
      on_subscribe_request(dynamic_cast<SubscribeRequestMessage &>(*message));
      break;
    case InvokeRequest:
      on_invoke_request(dynamic_cast<InvokeRequestMessage &>(*message));
      break;
    case SetRequest:
      on_set_request(dynamic_cast<SetRequestMessage &>(*message));
      break;
    case ListRequest:
      on_list_request(dynamic_cast<ListRequestMessage &>(*message));
      break;
    default:
      return;
  }
}

}
