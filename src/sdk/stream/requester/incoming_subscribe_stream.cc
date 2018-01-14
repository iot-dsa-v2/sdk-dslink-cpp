#include "dsa_common.h"

#include "incoming_subscribe_stream.h"

#include "core/session.h"
#include "message/request/subscribe_request_message.h"
#include "message/response/subscribe_response_message.h"

namespace dsa {

IncomingSubscribeStream::IncomingSubscribeStream(ref_<Session>&& session,
                                                 const Path& path, uint32_t rid,
                                                 Callback&& callback)
    : MessageCacheStream(std::move(session), path, rid),
      _callback(std::move(callback)) {}

void IncomingSubscribeStream::receive_message(ref_<Message>&& msg) {
  if (msg->type() == MessageType::SUBSCRIBE_RESPONSE) {
    IncomingPagesMerger::check_merge(_waiting_pages, msg);
    if (_callback != nullptr) {
      _callback(*this, std::move(msg));
    }
  }
}

void IncomingSubscribeStream::subscribe(const SubscribeOptions& options) {
  _options = options;
  auto msg = make_ref_<SubscribeRequestMessage>();
  msg->set_subscribe_option(options);
  msg->set_target_path(path.full_str());
  send_message(std::move(msg));
}

void IncomingSubscribeStream::close() {
  if (_closed) return;
  _closed = true;
  _callback = nullptr;
  // TODO: maybe wait to send message before destroying it
  send_message(make_ref_<RequestMessage>(MessageType::CLOSE_REQUEST), true);
}

bool IncomingSubscribeStream::check_close_message(MessageCRef& message) {
  if (message->type() == MessageType::CLOSE_REQUEST) {
    _session->requester.remove_stream(rid);
    return true;
  }
  return false;
}

void IncomingSubscribeStream::update_response_status(MessageStatus status) {
  if (_callback != nullptr) {
    auto response = make_ref_<SubscribeResponseMessage>();
    response->set_status(status);
    _callback(*this, std::move(response));
  }
}

bool IncomingSubscribeStream::disconnected() {
  update_response_status(MessageStatus::NOT_AVAILABLE);
  return false;
}
void IncomingSubscribeStream::reconnected() {
  if (!_writing) {
    subscribe(_options);
  }
}
}
