#include "dsa_common.h"

#include "incoming_list_stream.h"

#include "core/session.h"
#include "message/request/list_request_message.h"
#include "message/response/list_response_message.h"
#include "module/logger.h"

namespace dsa {

IncomingListStream::IncomingListStream(ref_<Session>&& session,
                                       const Path& path, uint32_t rid,
                                       Callback&& callback)
    : MessageCacheStream(std::move(session), path, rid),
      _callback(std::move(callback)) {}

void IncomingListStream::receive_message(ref_<Message>&& msg) {
  if (msg->type() == MessageType::LIST_RESPONSE) {
    if (_callback != nullptr) {
      BEFORE_CALLBACK_RUN();
      _callback(*this, std::move(msg));
      AFTER_CALLBACK_RUN();
    }
  }
}
void IncomingListStream::list(const ListOptions& options) {
  _options = options;
  auto msg = make_ref_<ListRequestMessage>();
  msg->set_list_option(options);
  msg->set_target_path(path.full_str());
  send_message(std::move(msg));
}

void IncomingListStream::close() {
  if (_closed) return;
  _closed = true;
  if (!_callback_running) {
    _callback = nullptr;
  }
  send_message(make_ref_<RequestMessage>(MessageType::CLOSE_REQUEST), true);
}

bool IncomingListStream::check_close_message(MessageCRef& message) {
  if (message->type() == MessageType::CLOSE_REQUEST) {
    _session->destroy_req_stream(rid);
    return true;
  }
  return false;
}

void IncomingListStream::update_response_status(Status status) {
  if (_callback != nullptr) {
    auto response = make_ref_<ListResponseMessage>();
    response->set_status(status);
    BEFORE_CALLBACK_RUN();
    _callback(*this, std::move(response));
    AFTER_CALLBACK_RUN();
  }
}

bool IncomingListStream::disconnected() {
  update_response_status(Status::NOT_AVAILABLE);
  return false;
}
void IncomingListStream::reconnected() {
  if (!_writing) {
    list(_options);
  }
}
}
