#include "dsa_common.h"

#include "incoming_subscribe_stream.h"

#include "message/response/subscribe_response_message.h"

namespace dsa {
IncomingSubscribeStream::IncomingSubscribeStream(ref_<Session>&& session,
                                                 uint32_t rid)
    : MessageCacheStream(std::move(session), rid) {}

void IncomingSubscribeStream::receive_message(MessageRef&& msg) {
  if (msg->type() == MessageType::SUBSCRIBE_RESPONSE) {
    if (_callback != nullptr) {
      _callback(DOWN_CAST<SubscribeResponseMessage*>(msg.get()));
    }
  } else if (msg->type() == MessageType::CLOSE) {
    if (_callback != nullptr) {
      // callback with nullptr
      _callback(ref_<SubscribeResponseMessage>());
    }
    close();
  }
}
}
