#include "dsa_common.h"

#include "outgoing_set_stream.h"

#include "core/session.h"
#include "message/request/set_request_message.h"
#include "module/logger.h"

namespace dsa {

OutgoingSetStream::OutgoingSetStream(ref_<Session> &&session, const Path &path,
                                     uint32_t rid,
                                     ref_<SetRequestMessage> &&message)
    : MessageCacheStream(std::move(session), path, rid),
      _waiting_request(std::move(message)) {}

void OutgoingSetStream::destroy_impl() {
  if (_callback != nullptr) {
    std::move(_callback)(*this, ref_<SetRequestMessage>());
  }
}

void OutgoingSetStream::receive_message(ref_<Message> &&message) {
  IncomingPagesMerger::check_merge(_waiting_pages, message);
  if (_callback != nullptr) {
    _callback(*this, std::move(message));
  } else {
    _waiting_request = std::move(message);
  }
};

void OutgoingSetStream::on_request(Callback &&callback) {
  _callback = std::move(callback);
  if (_callback != nullptr && _waiting_request != nullptr) {
    _callback(*this, std::move(_waiting_request));
  }
}

void OutgoingSetStream::send_response(
    ref_<const SetResponseMessage> &&message) {
  if (_closed) return;
  _closed = true;
  _callback = nullptr;
  if (message->get_status() < MessageStatus::CLOSED) {
    LOG_ERROR(_session->get_strand()->logger(),
              LOG << "set response must have closed or error status");
  }
  send_message(MessageCRef(std::move(message)), true);
};

bool OutgoingSetStream::check_close_message(MessageCRef &message) {
  if (DOWN_CAST<const ResponseMessage *>(message.get())->get_status() >=
      MessageStatus::CLOSED) {
    _session->responder.destroy_stream(rid);
    return true;
  }
  return false;
}
}