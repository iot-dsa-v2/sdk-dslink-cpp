#include "dsa_common.h"

#include "incoming_subscribe_stream.h"

#include "message/response/subscribe_response_message.h"

namespace dsa {

const SubscribeOptions IncomingSubscribeStream::default_options;

IncomingSubscribeStream::IncomingSubscribeStream(ref_<Session>&& session,
                                                 const std::string& path,
                                                 uint32_t rid,
                                                 Callback&& callback)
    : MessageCacheStream(std::move(session), path, rid),
      _callback(std::move(callback)) {}

void IncomingSubscribeStream::receive_message(MessageCRef&& msg) {
  if (msg->type() == MessageType::SUBSCRIBE_RESPONSE) {
    if (_callback != nullptr) {
      _callback(ref_cast_<const SubscribeResponseMessage>(msg), *this);
    }
  }
}
}
