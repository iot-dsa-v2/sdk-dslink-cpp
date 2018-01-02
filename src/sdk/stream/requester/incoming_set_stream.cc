#include "dsa_common.h"

#include "incoming_set_stream.h"

#include "core/session.h"
#include "message/request/set_request_message.h"
#include "message/response/set_response_message.h"

namespace dsa {

IncomingSetStream::IncomingSetStream(ref_<Session>&& session, const Path& path,
                                     uint32_t rid, Callback&& callback)
    : MessageCacheStream(std::move(session), path, rid),
      _callback(std::move(callback)) {}

void IncomingSetStream::receive_message(ref_<Message>&& msg) {
  if (msg->type() == MessageType::SET_RESPONSE) {
    if (_callback != nullptr) {
      _callback(*this, std::move(msg));
    }
  }
}

void IncomingSetStream::set(ref_<const SetRequestMessage>&& msg) {
  send_message(MessageCRef(std::move(msg)));
}

void IncomingSetStream::close() {
  if (_closed) return;
  _closed = true;
  _callback = nullptr;
  send_message(make_ref_<RequestMessage>(MessageType::CLOSE_REQUEST), true);
}

bool IncomingSetStream::check_close_message(MessageCRef& message) {
  if (message->type() == MessageType::CLOSE_REQUEST) {
    _session->requester.remove_stream(rid);
    return true;
  }
  return false;
}

bool IncomingSetStream::connection_changed() {
  if (_callback != nullptr) {
    auto response = make_ref_<SetResponseMessage>();
    response->set_status(MessageStatus::DISCONNECTED);
    _callback(*this, std::move(response));
  }
  destroy();
  return true;
}
}
