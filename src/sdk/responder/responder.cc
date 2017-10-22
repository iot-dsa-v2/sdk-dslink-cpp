#include "dsa_common.h"

#include "responder.h"

#include "module/security_manager.h"

#include "core/session.h"

#include "message/request/invoke_request_message.h"
#include "message/request/list_request_message.h"
#include "message/request/set_request_message.h"
#include "message/request/subscribe_request_message.h"
#include "stream/error_stream.h"
#include "stream/responder/outgoing_invoke_stream.h"
#include "stream/responder/outgoing_list_stream.h"
#include "stream/responder/outgoing_set_stream.h"
#include "stream/responder/outgoing_subscribe_stream.h"

namespace dsa {

Responder::Responder(Session &session) : _session(session) {}

void Responder::destroy_impl() { _outgoing_streams.clear(); }

void Responder::receive_message(ref_<Message> &&message) {
  auto find_stream = _outgoing_streams.find(message->get_rid());
  if (find_stream != _outgoing_streams.end()) {
    if (message->type() == MessageType::CLOSE) {
      find_stream->second->destroy();
      _outgoing_streams.erase(find_stream);
    } else {
      find_stream->second->receive_message(std::move(message));
    }
    return;
  }
  if (message->type() == MessageType::CLOSE) {
    // no need to close a stream that doesn't exist
    return;
  }
  auto request = DOWN_CAST<RequestMessage *>(message.get());
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
      case MessageType::INVOKE_REQUEST:
        on_invoke_request(ref_<InvokeRequestMessage>(std::move(message)));

        break;
      case MessageType::SUBSCRIBE_REQUEST:
        on_subscribe_request(ref_<SubscribeRequestMessage>(std::move(message)));
        break;
      case MessageType::LIST_REQUEST:
        on_list_request(ref_<ListRequestMessage>(std::move(message)));

        break;
      case MessageType::SET_REQUEST:
        on_set_request(ref_<SetRequestMessage>(std::move(message)));

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
  auto stream = make_ref_<OutgoingListStream>(
      _session.get_ref(), message->get_target_path(), message->get_rid(),
      message->get_list_options());

  _outgoing_streams[stream->rid] = stream;

  _session._strand->stream_acceptor().add(std::move(stream));
}

void Responder::on_invoke_request(ref_<InvokeRequestMessage> &&message) {
  auto stream = make_ref_<OutgoingInvokeStream>(
      _session.get_ref(), message->get_target_path(), message->get_rid(),
      std::move(message));

  _outgoing_streams[stream->rid] = stream;

  _session._strand->stream_acceptor().add(std::move(stream));
}

void Responder::on_set_request(ref_<SetRequestMessage> &&message) {
  auto stream = make_ref_<OutgoingSetStream>(
      _session.get_ref(), message->get_target_path(), message->get_rid(),
      std::move(message));

  _outgoing_streams[stream->rid] = stream;

  _session._strand->stream_acceptor().add(std::move(stream));
}

bool Responder::remove_stream(int32_t rid) {
  auto search = _outgoing_streams.find(rid);
  if (search != _outgoing_streams.end()) {
    auto &stream = search->second;
    stream->destroy();
    _outgoing_streams.erase(search);
    return true;
  }
  return false;
}

}  // namespace dsa
