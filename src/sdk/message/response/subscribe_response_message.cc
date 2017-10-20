#include "dsa_common.h"

#include "subscribe_response_message.h"

#include "util/date_time.h"

namespace dsa {
SubscribeResponseMessage::SubscribeResponseMessage(const uint8_t* data,
                                                   size_t size)
    : ResponseMessage(data, size) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
}

SubscribeResponseMessage::SubscribeResponseMessage()
    : ResponseMessage(MessageType::SUBSCRIBE_RESPONSE) {}

SubscribeResponseMessage::SubscribeResponseMessage(Var&& value)
    : ResponseMessage(MessageType::SUBSCRIBE_RESPONSE) {
  set_value(MessageValue(std::move(value), DateTime::get_ts()));
}

MessageValue SubscribeResponseMessage::get_value() const {
  return MessageValue(body->data(), body->size());
}
void SubscribeResponseMessage::set_value(MessageValue&& value) {
  body = value.to_msgpack();

  // invalidate message_size
  static_headers.message_size = 0;
}
}  // namespace dsa
