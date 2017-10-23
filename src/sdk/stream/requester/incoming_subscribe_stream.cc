#include "dsa_common.h"

#include "incoming_subscribe_stream.h"

#include "core/session.h"
#include "message/request/subscribe_request_message.h"
#include "message/response/subscribe_response_message.h"

namespace dsa {

const SubscribeOptions IncomingSubscribeStream::default_options;

IncomingSubscribeStream::IncomingSubscribeStream(ref_<Session>&& session,
                                                 const Path& path, uint32_t rid,
                                                 Callback&& callback)
    : MessageCacheStream(std::move(session), path, rid),
      _callback(std::move(callback)) {}

void IncomingSubscribeStream::receive_message(MessageCRef&& msg) {
  if (msg->type() == MessageType::SUBSCRIBE_RESPONSE) {
    if (_callback != nullptr) {
      _callback(*this, std::move(msg));
    }
  }
}

void IncomingSubscribeStream::subscribe(const SubscribeOptions& options) {
  auto msg = make_ref_<SubscribeRequestMessage>();
  msg->set_subscribe_option(options);
  msg->set_target_path(path.full_str());
  send_message(std::move(msg));
}

void IncomingSubscribeStream::close() {
  if (_closed) return;
  _closed = true;
  _callback = nullptr;
  send_message(make_ref_<RequestMessage>(MessageType::CLOSE), true);
}

bool IncomingSubscribeStream::check_close_message(MessageCRef& message) {
  if (message->type() == MessageType::CLOSE) {
    _session->requester.remove_stream(rid);
    return true;
  }
  return false;
}
}
