#include "dsa_common.h"

#include "subscribe_response_message.h"

namespace dsa {
SubscribeResponseMessage::SubscribeResponseMessage(const uint8_t* data, size_t size)
    : ResponseMessage(data, size) {
  parse_dynamic_headers(data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

SubscribeResponseMessage::SubscribeResponseMessage()
    : ResponseMessage(MessageType::SubscribeResponse) {}

const MessageValue& SubscribeResponseMessage::get_value() {
  if (_parsed_value == nullptr && body != nullptr) {
    _parsed_value.reset(new MessageValue(body->data(), body->size()));
  }
  return *_parsed_value;
}
void SubscribeResponseMessage::set_value(const Variant & value, const Variant & meta) {
  // TODO: create encode both meta and value, and create a Buffer Object to store the value
  // Buffer format  2bytes(meta size) + meta msgpack + value msgpack
}
}  // namespace dsa
