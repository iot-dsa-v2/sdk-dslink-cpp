#include "dsa_common.h"

#include "responder.h"

#include <functional>

#include "message/message_factory.h"
#include "message/request/invoke_request_message.h"
#include "message/request/list_request_message.h"
#include "message/request/set_request_message.h"
#include "message/request/subscribe_request_message.h"
#include "core/client.h"
#include "network/connection/tcp_connection.h"

namespace dsa {

void Responder::add_ready_outgoing_stream(uint32_t rid, size_t unique_id) {
  //_strand.post(make_intrusive_this_lambda([=]() {
  //  _ready_streams.push(StreamInfo{ rid, unique_id, &_outgoing_streams });
  //  if (!_is_writing) {
  //    _strand.post(boost::bind(&Session::write_loop, intrusive_this()));
  //  }
  //}));
}

bool Responder::add_outgoing_subscription(const intrusive_ptr_<OutgoingMessageStream> &stream) {
  if (_outgoing_streams.count(stream->_request_id) > 0)
    return false;

  {
    _outgoing_streams[stream->_request_id] = stream;
  }
  return true;
}

void Responder::remove_outgoing_subscription(uint32_t request_id) {
  _outgoing_streams.erase(request_id);
}


//
//Responder::Responder(const App &app, Config config)
//    :  _state_manager(app), _config(config) {}
//
//void Responder::start() {
//
//  // initialize new connection
//  ClientPtr connection = _initialize_connection();
//
//  // connect to broker
//  connection->connect();
//}
//
//ClientPtr Responder::_initialize_connection() {
//  auto on_connect = [this](const intrusive_ptr_<Session> session) {
//    _session = session;
//    _session->start();
//  };
//
//  //Config connection_config(_config.broker_hostname,
//  //                                     _config.broker_port, on_connect);
//  //ClientPtr connection(new TcpClientConnection(_app->shared_from_this(), connection_config));
//  //connection->set_message_handler(
//  //    std::bind(&Responder::_message_handler, share_this<Responder>(),
//  //              std::placeholders::_1, std::placeholders::_2));
//
//  return ClientPtr();
//}
//
//void Responder::_message_handler(const intrusive_ptr_<Session> &session,
//                                 SharedBuffer buf) {
//  std::unique_ptr<Message> message(parse_message(buf));
//  switch (message->type()) {
//    case SubscribeRequest:on_subscribe_request(dynamic_cast<SubscribeRequestMessage &>(*message));
//      break;
//    case InvokeRequest:on_invoke_request(dynamic_cast<InvokeRequestMessage &>(*message));
//      break;
//    case SetRequest:on_set_request(dynamic_cast<SetRequestMessage &>(*message));
//      break;
//    case ListRequest:on_list_request(dynamic_cast<ListRequestMessage &>(*message));
//      break;
//    default:return;
//  }
//}
//
//void Responder::on_subscribe_request(SubscribeRequestMessage &message) {
//  auto stream = make_shared_<SubscribeMessageStream>(_session,
//                                                         message.get_subscribe_options(),
//                                                         _stream_count++,
//                                                         message.request_id());
//  auto node_state = _state_manager.get_or_create(message.get_target_path());
//  node_state->add_subscription_stream(stream);
//  _session->add_outgoing_subscription(stream);
//
//  // let model manager find and attach the node model
//  _model_manager.find_model(node_state);
//}
//
//void Responder::on_invoke_request(InvokeRequestMessage &message) {
//  auto model = _model_manager.get_model(message.get_target_path());
//  if (model == nullptr)
//    return; // TODO: this should respond to the requester with an error
//  
//}
//
//void Responder::on_set_request(SetRequestMessage &message) {
//  // TODO: implement this
//}
//
//void Responder::on_list_request(ListRequestMessage &message) {
//  // TODO: implement this
//}

}
