#include "dsa_common.h"

#include "core/session.h"

#include "message/request/invoke_request_message.h"
#include "message/request/list_request_message.h"
#include "message/request/set_request_message.h"
#include "message/request/subscribe_request_message.h"
#include "module/authorizer.h"
#include "stream/responder/outgoing_invoke_stream.h"
#include "stream/responder/outgoing_list_stream.h"
#include "stream/responder/outgoing_set_stream.h"
#include "stream/responder/outgoing_subscribe_stream.h"
#include "stream/simple_stream.h"
#include "node_state_manager.h"

namespace dsa {

void Session::responder_disconnected() {
  for (auto it = _outgoing_streams.begin(); it != _outgoing_streams.end();
       ++it) {
    it->second->disconnected();
  }
  _outgoing_streams.clear();
}

inline ref_<OutgoingSubscribeStream> Session::on_subscribe_request(
    ref_<SubscribeRequestMessage> &&message) {
  return make_ref_<OutgoingSubscribeStream>(
      get_ref(), message->get_target_path(), message->get_rid(),
      message->get_subscribe_options());
}

inline ref_<OutgoingListStream> Session::on_list_request(
    ref_<ListRequestMessage> &&message) {
  return make_ref_<OutgoingListStream>(
      get_ref(), message->get_target_path(), message->get_rid(),
      message->get_list_options());
}

inline ref_<OutgoingInvokeStream> Session::on_invoke_request(
    ref_<InvokeRequestMessage> &&message) {
  return make_ref_<OutgoingInvokeStream>(
      get_ref(), message->get_target_path(), message->get_rid(),
      std::move(message));
}

inline ref_<OutgoingSetStream> Session::on_set_request(
    ref_<SetRequestMessage> &&message) {
  return make_ref_<OutgoingSetStream>(get_ref(),
                                      message->get_target_path(),
                                      message->get_rid(), std::move(message));
}

void Session::receive_resp_message(ref_<Message> &&message) {
  auto find_stream = _outgoing_streams.find(message->get_rid());
  if (find_stream != _outgoing_streams.end()) {
    if (message->type() == MessageType::CLOSE_REQUEST) {
      find_stream->second->destroy();
      _outgoing_streams.erase(find_stream);
    } else {
      find_stream->second->receive_message(std::move(message));
    }
    return;
  }
  if (message->type() == MessageType::CLOSE_REQUEST) {
    // no need to close a stream that doesn't exist
    return;
  }
  auto request = DOWN_CAST<RequestMessage *>(message.get());
  if (request->get_target_path().is_invalid()) {
    MessageType response_type = Message::get_response_type(request->type());
    if (response_type != MessageType::INVALID) {
      write_stream(make_ref_<SimpleStream>(
          request->get_rid(), response_type, Status::INVALID_MESSAGE));
    }
    return;
  }

  std::function<void(PermissionLevel)> callback;

  switch (message->type()) {
    case MessageType::INVOKE_REQUEST: {
      auto stream =
          on_invoke_request(ref_<InvokeRequestMessage>(message->get_ref()));
      callback = [
        stream, this,
        max_permission = static_cast<InvokeRequestMessage *>(message.get())
                             ->get_max_permission()
      ](PermissionLevel permission_level) mutable {
        // it's possible stream is closed before permission check
        if (stream->is_destroyed()) return;
        if (max_permission != PermissionLevel::NONE &&
            max_permission < permission_level) {
          permission_level = max_permission;
        }
        stream->allowed_permission = permission_level;

        if (permission_level < PermissionLevel::READ) {
          auto response = make_ref_<InvokeResponseMessage>();
          response->set_status(Status::PERMISSION_DENIED);
          stream->send_response(std::move(response));
          return;
        } else {
          _strand->stream_acceptor().add(std::move(stream));
        }
      };
      _outgoing_streams[stream->rid] = std::move(stream);
      break;
    }
    case MessageType::SUBSCRIBE_REQUEST: {
      auto stream = on_subscribe_request(
          ref_<SubscribeRequestMessage>(message->get_ref()));
      callback = [stream, this](PermissionLevel permission_level) mutable {
        // it's possible stream is closed before permission check
        if (stream->is_destroyed()) return;

        stream->allowed_permission = permission_level;

        if (permission_level < PermissionLevel::READ) {
          auto response = make_ref_<SubscribeResponseMessage>();
          response->set_status(Status::PERMISSION_DENIED);
          stream->send_subscribe_response(std::move(response));
          return;
        } else {
          _strand->stream_acceptor().add(std::move(stream));
        }
      };
      _outgoing_streams[stream->rid] = std::move(stream);
      break;
    }
    case MessageType::LIST_REQUEST: {
      auto stream =
          on_list_request(ref_<ListRequestMessage>(message->get_ref()));
      callback = [stream, this](PermissionLevel permission_level) mutable {
        // it's possible stream is closed before permission check
        if (stream->is_destroyed()) return;

        stream->allowed_permission = permission_level;

        if (permission_level < PermissionLevel::LIST) {
          stream->update_response_status(Status::PERMISSION_DENIED);
          return;
        } else {
          _strand->stream_acceptor().add(std::move(stream));
        }
      };
      _outgoing_streams[stream->rid] = std::move(stream);
      break;
    }
    case MessageType::SET_REQUEST: {
      auto stream = on_set_request(ref_<SetRequestMessage>(message->get_ref()));
      callback = [stream, this](PermissionLevel permission_level) mutable {
        // it's possible stream is closed before permission check
        if (stream->is_destroyed()) return;

        stream->allowed_permission = permission_level;

        if (permission_level < PermissionLevel::WRITE) {
          stream->close(Status::PERMISSION_DENIED);
          return;
        } else {
          _strand->stream_acceptor().add(std::move(stream));
        }
      };
      _outgoing_streams[stream->rid] = std::move(stream);
      break;
    }

    default:
      return;
  }

  _strand->authorizer().check_permission(
      get_remote_id(), _role, request->get_permission_token(),
      request->type(), request->get_target_path(), std::move(callback));
}

bool Session::destroy_resp_stream(int32_t rid) {
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
