#include "dsa_common.h"

#include "outgoing_invoke_stream.h"

#include "core/session.h"
#include "message/request/invoke_request_message.h"
#include "module/logger.h"

namespace dsa {

OutgoingInvokeStream::OutgoingInvokeStream(ref_<Session> &&session,
                                           const Path &path, uint32_t rid,
                                           ref_<InvokeRequestMessage> &&message)
    : MessageQueueStream(std::move(session), path, rid) {
  if (message->get_page_id() < 0) {
    _waiting_pages = make_ref_<IncomingPagesMerger>(message);
  }
  _waiting_requests.emplace_back(std::move(message));
}

void OutgoingInvokeStream::destroy_impl() {
  if (_callback != nullptr && !_callback_running) {
    _callback(*this, ref_<InvokeRequestMessage>());
    _callback = nullptr;
  }
  MessageQueueStream::destroy_impl();
}

void OutgoingInvokeStream::receive_message(ref_<Message> &&message) {
  if (message != nullptr) {
    IncomingPagesMerger::check_merge(_waiting_pages, message);
  }
  if (_callback != nullptr) {
    BEFORE_CALLBACK_RUN();
    _callback(*this, std::move(message));
    AFTER_CALLBACK_RUN();
  } else {
    _waiting_requests.emplace_back(std::move(message));
  }
};

void OutgoingInvokeStream::on_request(Callback &&callback) {
  _callback = std::move(callback);
  if (_callback != nullptr && !_waiting_requests.empty()) {
    BEFORE_CALLBACK_RUN();
    for (auto &msg : _waiting_requests) {
      _callback(*this, std::move(msg));
    }
    AFTER_CALLBACK_RUN();
    _waiting_requests.clear();
  }
}
void OutgoingInvokeStream::send_response(InvokeResponseMessageCRef &&message) {
  if (message->get_status() >= MessageStatus::CLOSED && !_closed) {
    _closed = true;
    if (!_callback_running) {
      _callback = nullptr;
    }
    send_message(MessageCRef(std::move(message)), true);
  } else {
    send_message(MessageCRef(std::move(message)));
  }
};
void OutgoingInvokeStream::close(MessageStatus status,
                                 const string_ &err_detail) {
  if (_closed) return;
  if (status < MessageStatus::CLOSED) {
    status = MessageStatus::CLOSED;
  }
  _closed = true;
  if (!_callback_running) {
    _callback = nullptr;
  }

  auto message = make_ref_<InvokeResponseMessage>();
  message->set_status(status);
  if (!err_detail.empty()) {
    // TODO general error detail
  }
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