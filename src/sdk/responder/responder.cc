#include "dsa_common.h"

#include "responder.h"

#include "module/security_manager.h"

#include "core/session.h"

#include "message/request/invoke_request_message.h"
#include "message/request/list_request_message.h"
#include "message/request/set_request_message.h"
#include "message/request/subscribe_request_message.h"

namespace dsa {

Responder::Responder(Session &session) : _session(session) {}

void Responder::receive_message(ref_<Message> &&message) {
  auto request = DOWN_CAST<RequestMessage *>(message.get());
  auto callback = [ message = std::move(message),
                    this ](PermissionLevel permission) mutable {
    // TODO: implement permissions

    switch (message->type()) {
      case MessageType::SUBSCRIBE_REQUEST:
        on_subscribe_request(ref_<SubscribeRequestMessage>(
            DOWN_CAST<SubscribeRequestMessage *>(message.get())));
        break;
      case MessageType::INVOKE_REQUEST:
        on_invoke_request(ref_<InvokeRequestMessage>(
            DOWN_CAST<InvokeRequestMessage *>(message.get())));

        break;
      case MessageType::SET_REQUEST:
        on_set_request(ref_<SetRequestMessage>(
            DOWN_CAST<SetRequestMessage *>(message.get())));

        break;
      case MessageType::LIST_REQUEST:
        on_list_request(ref_<ListRequestMessage>(
            DOWN_CAST<ListRequestMessage *>(message.get())));

        break;
      default:
        return;
    }
  };

  _session._strand->security_manager().check_permission(
      _session.dsid(), request->get_permission_token(), request->type(),
      request->get_target_path(), std::move(callback));
}

void Responder::on_subscribe_request(
    ref_<SubscribeRequestMessage> &&message) {
  auto stream = make_ref_<SubscribeMessageStream>(
    _session.get_intrusive(), message->get_subscribe_options(),
    message->request_id());

  auto node_state = _session._strand->state_manager().get_or_create(
      message->get_target_path());
  node_state->add_stream(stream);

  //  if (!node_state->has_model())
  //    _model_manager.find_model(node_state);
}

void Responder::on_list_request(ref_<ListRequestMessage> &&message) {
  auto stream = make_ref_<ListMessageStream>(_session.get_intrusive(),
                                             message->get_list_options(),
                                             message->request_id());

  auto node_state = _session._strand->state_manager().get_or_create(
      message->get_target_path());
  node_state->add_stream(stream);

  //  if (!node_state->has_model())
  //    _model_manager.find_model(node_state);
}

void Responder::on_invoke_request(
    ref_<InvokeRequestMessage> &&message) {
  //  auto model = _model_manager.get_model(message.get_target_path());
  //  if (model == nullptr) {
  //    send_error(MessageType::INVOKE_RESPONSE, MessageStatus::DISCONNECTED,
  //    message.request_id());
  //    return;
  //  }
  //
  //  auto stream =
  //  make_ref_<InvokeMessageStream>(_session.get_intrusive(),
  //                                                     message.get_invoke_options(),
  //                                                     message.request_id(),
  //                                                     );
  //  model->add_stream(stream);
}

void Responder::on_set_request(ref_<SetRequestMessage> &&message) {
  //  auto model = _model_manager.get_model(message.get_target_path());
  //  if (model == nullptr) {
  //    send_error(MessageType::SUBSCRIBE_RESPONSE, MessageStatus::DISCONNECTED,
  //    message.request_id());
  //    return;
  //  }
  //
  //  auto stream = make_ref_<SetMessageStream>(_session.get_intrusive(),
  //                                                  message.get_set_options(),
  //                                                  message.request_id(),
  //                                                  );
  //  model->add_stream(stream);
}
}  // namespace dsa
