#include "dsa_common.h"

#include "outgoing_invoke_stream.h"

#include "core/session.h"
#include "message/request/invoke_request_message.h"

namespace dsa {

OutgoingInvokeStream::OutgoingInvokeStream(
    ref_<Session> &&session, const Path &path, uint32_t rid,
    ref_<const InvokeRequestMessage> &&mesage)
    : MessageQueueStream(std::move(session), path, rid) {
  _waiting_requests.emplace_back(std::move(mesage));
}

void OutgoingInvokeStream::destroy_impl() {
  if (_callback != nullptr) {
    std::move(_callback)(*this, ref_<const InvokeRequestMessage>());
  }
  MessageQueueStream::destroy_impl();
}

void OutgoingInvokeStream::receive_message(MessageCRef &&mesage) {
  if (_callback != nullptr) {
    _callback(*this, std::move(mesage));
  } else {
    _waiting_requests.emplace_back(std::move(mesage));
  }
};

void OutgoingInvokeStream::on_request(Callback &&callback) {
  _callback = std::move(callback);
  if (_callback != nullptr && !_waiting_requests.empty()) {
    for (auto &msg : _waiting_requests) {
      _callback(*this, std::move(msg));
    }
    _waiting_requests.clear();
  }
}

void OutgoingInvokeStream::close(MessageStatus status) {
  if (_closed) return;
  if (status < MessageStatus::CLOSED) {
    status = MessageStatus::CLOSED;
  }
  _closed = true;
  _callback = nullptr;

  auto message = make_ref_<InvokeResponseMessage>();
  message->set_status(status);
  send_message(std::move(message), true);
}

MessageCRef OutgoingInvokeStream::get_next_message(AckCallback &callback) {
  _writing = false;
  if (is_destroyed() || _queue.empty()) {
    return MessageCRef();
  }
  MessageCRef msg = std::move(_queue.front());
  _queue.pop_front();
  _current_queue_size -= msg->size();

  if (DOWN_CAST<const ResponseMessage *>(msg.get())->get_status() >=
      MessageStatus::CLOSED) {
    // clear the stream and return the cache
    MessageCRef copy = std::move(msg);
    _session->responder.remove_stream(rid);
    return std::move(copy);
  } else if (!_queue.empty()) {
    _current_queue_time = _queue.front()->created_ts;
    _writing = true;
    _session->write_stream(get_ref());
  }
  return std::move(msg);
}
}