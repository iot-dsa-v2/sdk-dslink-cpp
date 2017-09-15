#include "dsa_common.h"

#include "subscribe_response_message.h"

#include <ctime>
#include "util/date_time.h"

namespace dsa {
SubscribeResponseMessage::SubscribeResponseMessage(const uint8_t* data,
                                                   size_t size)
    : ResponseMessage(data, size), created_ts(std::time(nullptr)) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
}

SubscribeResponseMessage::SubscribeResponseMessage()
    : ResponseMessage(MessageType::SUBSCRIBE_RESPONSE),
      created_ts(std::time(nullptr)) {}

SubscribeResponseMessage::SubscribeResponseMessage(Variant&& value)
    : ResponseMessage(MessageType::SUBSCRIBE_RESPONSE),
      created_ts(std::time(nullptr)) {
  set_value(MessageValue(std::move(value), DateTime::get_ts()));
}

const MessageValue& SubscribeResponseMessage::get_value() const {
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
