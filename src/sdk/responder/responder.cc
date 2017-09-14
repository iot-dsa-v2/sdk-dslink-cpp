#include "dsa_common.h"

#include "responder.h"

#include "module/security_manager.h"

#include "core/session.h"

#include "message/request/invoke_request_message.h"
#include "message/request/list_request_message.h"
#include "message/request/set_request_message.h"
#include "message/request/subscribe_request_message.h"
#include "stream/error_stream.h"
#include "stream/responder/outgoing_subscribe_stream.h"

namespace dsa {

Responder::Responder(Session &session) : _session(session) {}

void Responder::receive_message(ref_<Message> &&message) {
  auto request = DOWN_CAST<RequestMessage *>(message.get());
  auto find_stream = _outgoing_streams.find(request->get_rid());
  if (find_stream != _outgoing_streams.end()) {
    if (request->type() == MessageType::CLOSE) {
      find_stream->second->close();
    } else {
      find_stream->second->receive_message(std::move(message));
    }
    return;
  }
  if (request->type() == MessageType::CLOSE) {
    // no need to close a stream that doesn't exist
    return;
  }
  if (request->get_target_path().is_invalid()) {
    MessageType response_type = Message::get_response_type(request->type());
    if (response_type != MessageType::INVALID) {
      _session.write_stream(make_ref_<ErrorStream>(
          request->get_rid(), response_type, MessageStatus::INVALID_MESSAGE));
    }
    return;
  }

  auto callback = [ message = std::move(message),
                    this ](PermissionLevel permission) mutable {
    // TODO: implement permissions

    switch (message->type()) {
      case MessageType::SUBSCRIBE_REQUEST:
        on_subscribe_request(ref_<SubscribeRequestMessage>(
            ref_cast_<SubscribeRequestMessage>(message)));
        break;
      case MessageType::INVOKE_REQUEST:
        on_invoke_request(ref_<InvokeRequestMessage>(
            ref_cast_<InvokeRequestMessage>(message)));

        break;
      case MessageType::SET_REQUEST:
        on_set_request(
            ref_<SetRequestMessage>(ref_cast_<SetRequestMessage>(message)));

        break;
      case MessageType::LIST_REQUEST:
        on_list_request(
            ref_<ListRequestMessage>(ref_cast_<ListRequestMessage>(message)));

        break;
      default:
        return;
    }
  };

  _session._strand->security_manager().check_permission(
      _session.dsid(), request->get_permission_token(), request->type(),
      request->get_target_path(), std::move(callback));
}

void Responder::on_subscribe_request(ref_<SubscribeRequestMessage> &&message) {
  auto stream = make_ref_<OutgoingSubscribeStream>(
      _session.get_ref(), message->get_target_path(), message->get_rid(),
      message->get_subscribe_options());

  _outgoing_streams[stream->rid] = stream;

  _session._strand->stream_acceptor().add(std::move(stream));
}

void Responder::on_list_request(ref_<ListRequestMessage> &&message) {
  //  auto stream = make_ref_<ListMessageStream>(_session.get_intrusive(),
  //                                             message->get_list_options(),
  //                                             message->get_rid());
  //
  //  auto node_state = _session._strand->state_manager().get_or_create(
  //      message->get_target_path());
  //  node_state->add_stream(stream);

  //  if (!node_state->has_model())
  //    _model_manager.find_model(node_state);
}

void Responder::on_invoke_request(ref_<InvokeRequestMessage> &&message) {
  //  auto model = _model_manager.get_model(message.get_target_path());
  //  if (model == nullptr) {
  //    send_error(MessageType::INVOKE_RESPONSE, MessageStatus::DISCONNECTED,
  //    message.get_rid());
  //    return;
  //  }
  //
  //  auto stream =
  //  make_ref_<InvokeMessageStream>(_session.get_intrusive(),
  //                                                     message.get_invoke_options(),
  //                                                     message.get_rid(),
  //                                                     );
  //  model->add_stream(stream);
}

void Responder::on_set_request(ref_<SetRequestMessage> &&message) {
  //  auto model = _model_manager.get_model(message.get_target_path());
  //  if (model == nullptr) {
  //    send_error(MessageType::SUBSCRIBE_RESPONSE, MessageStatus::DISCONNECTED,
  //    message.get_rid());
  //    return;
  //  }
  //
  //  auto stream = make_ref_<SetMessageStream>(_session.get_intrusive(),
  //                                                  message.get_set_options(),
  //                                                  message.get_rid(),
  //                                                  );
  //  model->add_stream(stream);
}
}  // namespace dsa
