#include "dsa_common.h"

#include "incoming_invoke_stream.h"

#include "core/session.h"
#include "message/request/invoke_request_message.h"
#include "message/response/invoke_response_message.h"

namespace dsa {

IncomingInvokeStream::IncomingInvokeStream(ref_<Session>&& session,
                                           const Path& path, uint32_t rid,
                                           Callback&& callback)
    : MessageQueueStream(std::move(session), path, rid),
      _callback(std::move(callback)) {}

void IncomingInvokeStream::receive_message(MessageCRef&& mesage) {
  if (mesage->type() == MessageType::INVOKE_RESPONSE) {
    if (_callback != nullptr) {
      if (DOWN_CAST<const InvokeResponseMessage*>(mesage.get())->get_status() >=
          MessageStatus::CLOSED) {
        _closed = true;
      }
      _callback(*this, std::move(mesage));
    }
  }
}

void IncomingInvokeStream::invoke(ref_<const InvokeRequestMessage>&& msg) {
  send_message(MessageCRef(std::move(msg)));
}

void IncomingInvokeStream::close() {
  if (_closed) return;
  _closed = true;
  _callback = nullptr;
  send_message(make_ref_<RequestMessage>(MessageType::CLOSE), true);
}

MessageCRef IncomingInvokeStream::get_next_message(AckCallback& callback) {
  _writing = false;
  if (is_destroyed() || _queue.empty()) {
    return MessageCRef();
  }
  MessageCRef msg = std::move(_queue.front());
  _queue.pop_front();
  _current_queue_size -= msg->size();
  if (msg->type() == MessageType::CLOSE) {
    // clear the stream and return the cache
    MessageCRef copy = std::move(msg);
    _session->requester.remove_stream(rid);
    return std::move(copy);
  } else if (!_queue.empty()) {
    _current_queue_time = _queue.front()->created_ts;
    _writing = true;
    _session->write_stream(get_ref());
  }
  return std::move(msg);
}
}
