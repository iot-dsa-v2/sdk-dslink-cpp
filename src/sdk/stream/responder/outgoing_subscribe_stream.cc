#include "dsa_common.h"

#include "outgoing_subscribe_stream.h"

#include "util/date_time.h"

namespace dsa {

OutgoingSubscribeStream::OutgoingSubscribeStream(ref_<Session> &&session,
                                                 const std::string &path,
                                                 uint32_t rid,
                                                 SubscribeOptions &&options)
    : OutgoingMessageStream(std::move(session), path, rid),
      options(std::move(options)) {}

void OutgoingSubscribeStream::receive_message(MessageRef &&message) {
  if (message->type() == MessageType::CLOSE) {
  } else {
  }
}

void OutgoingSubscribeStream::send_value(Variant &&value) {
  auto response = make_ref_<SubscribeResponseMessage>();
  response->set_value(MessageValue(std::move(value), DateTime::get_ts()));
  send_message(std::move(response));
}
void OutgoingSubscribeStream::send_value(MessageValue &&value) {
  auto response = make_ref_<SubscribeResponseMessage>();
  response->set_value(std::move(value));
  send_message(std::move(response));
}
}
