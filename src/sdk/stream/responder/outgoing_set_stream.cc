#include "dsa_common.h"

#include "outgoing_set_stream.h"

#include "core/session.h"
#include "message/request/set_request_message.h"
#include "module/logger.h"

namespace dsa {

OutgoingSetStream::OutgoingSetStream(ref_<Session> &&session, const Path &path,
                                     uint32_t rid,
                                     ref_<SetRequestMessage> &&message)
    : MessageCacheStream(std::move(session), path, rid) {
  if (message->get_page_id() < 0) {
    _waiting_pages = make_ref_<IncomingPagesMerger>(message);
  }
  _waiting_requests.emplace_back(std::move(message));
}

void OutgoingSetStream::destroy_impl() {
  if (_callback != nullptr && !_callback_running) {
    _callback(*this, ref_<SetRequestMessage>());
    _callback = nullptr;
  }
  MessageCacheStream::destroy_impl();
}

void OutgoingSetStream::receive_message(ref_<Message> &&message) {
  IncomingPagesMerger::check_merge(_waiting_pages, message);
  if (_callback != nullptr) {
    BEFORE_CALLBACK_RUN();
    _callback(*this, std::move(message));
    AFTER_CALLBACK_RUN();
  } else {
    _waiting_requests.emplace_back(std::move(message));
  }
};

void OutgoingSetStream::on_request(Callback &&callback) {
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

void OutgoingSetStream::send_response(
    ref_<const SetResponseMessage> &&message) {
  if (_closed) return;
  _closed = true;
  if (!_callback_running) {
    _callback = nullptr;
  }
  if (message->get_status() < MessageStatus::CLOSED) {
    LOG_ERROR(__FILENAME__,
              LOG << "set response must have closed or error status");
  }
  send_message(MessageCRef(std::move(message)), true);
};
void OutgoingSetStream::close(MessageStatus status, const string_ &err_detail) {
  if (_closed) return;
  if (status < MessageStatus::CLOSED) {
    status = MessageStatus::CLOSED;
  }
  _closed = true;
  if (!_callback_running) {
    _callback = nullptr;
  }

  auto message = make_ref_<SetResponseMessage>();
  message->set_status(status);
  if (!err_detail.empty()) {
    message->set_error_detail(err_detail);
  }
  send_message(std::move(message), true);
}

bool OutgoingSetStream::check_close_message(MessageCRef &message) {
  if (DOWN_CAST<const ResponseMessage *>(message.get())->get_status() >=
      MessageStatus::CLOSED) {
    _session->responder.destroy_stream(rid);
    return true;
  }
  return false;
}
}  // namespace dsa