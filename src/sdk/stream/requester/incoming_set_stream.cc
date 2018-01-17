#include "dsa_common.h"

#include "incoming_set_stream.h"

#include "core/session.h"
#include "message/request/set_request_message.h"
#include "message/response/set_response_message.h"
#include "module/logger.h"

namespace dsa {

IncomingSetStream::IncomingSetStream(ref_<Session>&& session, const Path& path,
                                     uint32_t rid, Callback&& callback)
    : MessageQueueStream(std::move(session), path, rid),
      _callback(std::move(callback)) {}

void IncomingSetStream::receive_message(ref_<Message>&& msg) {
  if (msg->type() == MessageType::SET_RESPONSE) {
    if (_callback != nullptr) {
      BEFORE_CALLBACK_RUN();
      _callback(*this, std::move(msg));
      AFTER_CALLBACK_RUN();
    }
  }
}

void IncomingSetStream::set(ref_<const SetRequestMessage>&& msg) {
  send_message(MessageCRef(std::move(msg)));
}

void IncomingSetStream::close() {
  if (_closed) return;
  _closed = true;
  if (!_callback_running) {
    _callback = nullptr;
  }
  send_message(make_ref_<RequestMessage>(MessageType::CLOSE_REQUEST), true);
}

bool IncomingSetStream::check_close_message(MessageCRef& message) {
  if (message->type() == MessageType::CLOSE_REQUEST) {
    _session->requester.remove_stream(rid);
    return true;
  }
  return false;
}

bool IncomingSetStream::disconnected() {
  if (_callback != nullptr) {
    auto response = make_ref_<SetResponseMessage>();
    response->set_status(MessageStatus::DISCONNECTED);
    BEFORE_CALLBACK_RUN();
    _callback(*this, std::move(response));
    AFTER_CALLBACK_RUN();
  }
  destroy();
  return true;
}
}
