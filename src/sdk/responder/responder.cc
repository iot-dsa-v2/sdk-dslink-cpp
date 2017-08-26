#include "dsa_common.h"

#include "responder.h"

#include "module/security_manager.h"


#include "core/session.h"

#include "message/request/invoke_request_message.h"
#include "message/request/list_request_message.h"
#include "message/request/set_request_message.h"
#include "message/request/subscribe_request_message.h"

namespace dsa {

Responder::Responder(Session &session)
    : _session(session) {}

void Responder::receive_message(intrusive_ptr_<RequestMessage> message) {
  auto callback = [message, this](PermissionLevel permission) {
    // TODO: implement permissions

    switch (message->type()) {
      case MessageType::SubscribeRequest:
        on_subscribe_request(dynamic_cast<SubscribeRequestMessage &>(*message));
        break;
      case MessageType::InvokeRequest:
        on_invoke_request(dynamic_cast<InvokeRequestMessage &>(*message));
        break;
      case MessageType::SetRequest:
        on_set_request(dynamic_cast<SetRequestMessage &>(*message));
        break;
      case MessageType::ListRequest:
        on_list_request(dynamic_cast<ListRequestMessage &>(*message));
        break;
      default:
        return;
    }
  };

  _session._strand->security_manager().check_permission(_session.dsid(),
                                      message->get_permission_token(),
                                      message->type(),
                                      message->get_target_path(),
                                      std::move(callback));
}

void Responder::send_error(MessageType &&type, MessageStatus &&status, uint32_t &&request_id) {
  _session.add_ready_stream(intrusive_ptr_<MessageStream>(
      new ErrorMessageStream(_session.get_intrusive(),
                             std::forward<MessageType>(type),
                             std::forward<MessageStatus>(status),
                             std::forward<uint32_t>(request_id))));
}

void Responder::on_subscribe_request(SubscribeRequestMessage &message) {
  auto stream = make_intrusive_<SubscribeMessageStream>(_session.get_intrusive(),
                                                        message.get_subscribe_options(),
                                                        message.request_id()
                                                        );

  auto node_state = _session._strand->state_manager().get_or_create(message.get_target_path());
  node_state->add_stream(stream);

//  if (!node_state->has_model())
//    _model_manager.find_model(node_state);
}

void Responder::on_list_request(ListRequestMessage &message) {
  auto stream = make_intrusive_<ListMessageStream>(_session.get_intrusive(),
                                                   message.get_list_options(),
                                                   message.request_id()
                                                   );

  auto node_state = _session._strand->state_manager().get_or_create(message.get_target_path());
  node_state->add_stream(stream);

//  if (!node_state->has_model())
//    _model_manager.find_model(node_state);
}

void Responder::on_invoke_request(InvokeRequestMessage &message) {
//  auto model = _model_manager.get_model(message.get_target_path());
//  if (model == nullptr) {
//    send_error(MessageType::InvokeResponse, MessageStatus::Disconnected, message.request_id());
//    return;
//  }
//
//  auto stream = make_intrusive_<InvokeMessageStream>(_session.get_intrusive(),
//                                                     message.get_invoke_options(),
//                                                     message.request_id(),
//                                                     );
//  model->add_stream(stream);
}

void Responder::on_set_request(SetRequestMessage &message) {
//  auto model = _model_manager.get_model(message.get_target_path());
//  if (model == nullptr) {
//    send_error(MessageType::SubscribeResponse, MessageStatus::Disconnected, message.request_id());
//    return;
//  }
//
//  auto stream = make_intrusive_<SetMessageStream>(_session.get_intrusive(),
//                                                  message.get_set_options(),
//                                                  message.request_id(),
//                                                  );
//  model->add_stream(stream);
}
}  // namespace dsa
