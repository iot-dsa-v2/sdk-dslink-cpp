#include "dsa_common.h"

#include "subscribe_response_message.h"

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

const MessageValue& SubscribeResponseMessage::get_value() {
  if (!_parsed && body != nullptr) {
    _parsed_value.parse(body->data(), body->size());
    _parsed = true;
  }
  return _parsed_value;
}
void SubscribeResponseMessage::set_value(MessageValue&& value) {
  _parsed_value = std::move(value);

  body = _parsed_value.to_msgpack();

  // invalidate message_size
  static_headers.message_size = 0;
}
}  // namespace dsa
