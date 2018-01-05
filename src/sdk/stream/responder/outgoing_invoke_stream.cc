#include "dsa_common.h"

#include "outgoing_invoke_stream.h"

#include "core/session.h"
#include "message/request/invoke_request_message.h"

namespace dsa {

OutgoingInvokeStream::OutgoingInvokeStream(ref_<Session> &&session,
                                           const Path &path, uint32_t rid,
                                           ref_<InvokeRequestMessage> &&mesage)
    : MessageQueueStream(std::move(session), path, rid) {
  if (mesage->get_page_id() < 0) {
    _waiting_pages = make_ref_<IncomingPages>(mesage);
  }
  _waiting_requests.emplace_back(std::move(mesage));
}

void OutgoingInvokeStream::destroy_impl() {
  if (_callback != nullptr) {
    std::move(_callback)(*this, ref_<InvokeRequestMessage>());
  }
  MessageQueueStream::destroy_impl();
}

void OutgoingInvokeStream::receive_message(ref_<Message> &&mesage) {
  if (mesage != nullptr) {
    if (mesage->get_page_id() < 0) {
      _waiting_pages = make_ref_<IncomingPages>(mesage);
    } else if (_waiting_pages != nullptr) {
      if (!_waiting_pages->check_add(mesage)) {
        // invalid page, drop the waiting pages
        _waiting_pages = nullptr;
      }
    }
  }
  if (_callback != nullptr) {
    _callback(*this, std::move(mesage));
  } else {
    _waiting_requests.emplace_back(std::move(mesage));
  }
};
MessageCRef OutgoingInvokeStream::get_first_page_when_ready() {
  if (_waiting_pages != nullptr && _waiting_pages->is_ready()) {
    return _waiting_pages->first;
  }
  return MessageCRef();
}
void OutgoingInvokeStream::on_request(Callback &&callback) {
  _callback = std::move(callback);
  if (_callback != nullptr && !_waiting_requests.empty()) {
    for (auto &msg : _waiting_requests) {
      _callback(*this, std::move(msg));
    }
    _waiting_requests.clear();
  }
}
void OutgoingInvokeStream::send_response(InvokeResponseMessageCRef &&message) {
  if (message->get_status() >= MessageStatus::CLOSED && !_closed) {
    _closed = true;
    _callback = nullptr;
    send_message(MessageCRef(std::move(message)), true);
  } else {
    send_message(MessageCRef(std::move(message)));
  }
};
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

bool OutgoingInvokeStream::check_close_message(MessageCRef &message) {
  if (DOWN_CAST<const ResponseMessage *>(message.get())->get_status() >=
      MessageStatus::CLOSED) {
    _session->responder.destroy_stream(rid);
    return true;
  }
  return false;
}
}