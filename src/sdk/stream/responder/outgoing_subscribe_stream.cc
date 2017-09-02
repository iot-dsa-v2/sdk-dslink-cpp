#include "dsa_common.h"

#include "outgoing_subscribe_stream.h"

namespace dsa {

OutgoingSubscribeStream::OutgoingSubscribeStream(ref_<Session> &&session,
                                                 const std::string &path,
                                                 uint32_t rid,
                                                 SubscribeOptions &&options)
    : OutgoingMessageStream(std::move(session), path, rid), options(std::move(options)) {}

void OutgoingSubscribeStream::new_message(
    const SubscribeResponseMessage &new_message) {}

void OutgoingSubscribeStream::receive_message(MessageRef &&message) {
  if (message->type() == MessageType::CLOSE) {

  } else {

  }

}
}
