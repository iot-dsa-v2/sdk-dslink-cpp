#include <dsa/message.h>
#include "dsa_common.h"

#include "responder.h"

#include "node_model_manager.h"
#include "module/security_manager.h"

#include "core/session.h"

namespace dsa {

Responder::Responder(Session &session, const Config &config)
    : _session(session),
      _strand(session.strand()),
      _security_manager(config.security_manager),
      _model_manager(config.model_manager),
      _state_manager(session.strand()) {}

void Responder::receive_message(intrusive_ptr_<RequestMessage> message) {
  auto callback = [message, this](PermissionLevel permission) {
    // TODO: implement permissions

    switch (message->type()) {
      case MessageType::SubscribeRequest:on_subscribe_request(dynamic_cast<SubscribeRequestMessage &>(*message));
        break;
      case MessageType::InvokeRequest:on_invoke_request(dynamic_cast<InvokeRequestMessage &>(*message));
        break;
      case MessageType::SetRequest:on_set_request(dynamic_cast<SetRequestMessage &>(*message));
        break;
      case MessageType::ListRequest:on_list_request(dynamic_cast<ListRequestMessage &>(*message));
        break;
      default:return;
    }
  };

  _security_manager->check_permission(_session.dsid(),
                                      message->get_permission_token(),
                                      message->type(),
                                      message->get_target_path(),
                                      std::move(callback));
}

void Responder::on_subscribe_request(SubscribeRequestMessage &message) {
  auto node_state = _state_manager.get_or_create(message.get_target_path());
  node_state->new_subscription_stream(_session.get_intrusive(),
                                      message.get_subscribe_options(),
                                      _stream_count++,
                                      message.request_id());

  if (!node_state->has_model())
    _model_manager->find_model(node_state);
}

void Responder::on_list_request(ListRequestMessage &message) {
  auto node_state = _state_manager.get_or_create(message.get_target_path());
  node_state->new_list_stream(_session.get_intrusive(),
                              message.get_list_options(),
                              _stream_count++,
                              message.request_id());

  if (!node_state->has_model())
    _model_manager->find_model(node_state);
}

void Responder::on_invoke_request(InvokeRequestMessage &message) {
  auto model = _model_manager->get_model(message.get_target_path());
  if (model == nullptr)
    return; // TODO: this should respond to the requester with an error

  model->new_invoke_stream(_session.get_intrusive(),
                           message.get_invoke_options(),
                           _stream_count++,
                           message.request_id());
}

void Responder::on_set_request(SetRequestMessage &message) {
  auto model = _model_manager->get_model(message.get_target_path());
  if (model == nullptr)
    return; // TODO: this should respond to the requester with an error

  model->new_set_stream(_session.get_intrusive(),
                        message.get_set_options(),
                        _stream_count++,
                        message.request_id());
}

void Responder::set_ready_stream(MessageStream::StreamInfo &&stream_info) {
  _session.add_ready_stream(std::move(stream_info));
}
}
