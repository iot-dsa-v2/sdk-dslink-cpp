#include "dsa_common.h"

#include "subscribe_response_message.h"

namespace dsa {
SubscribeResponseMessage::SubscribeResponseMessage(const SharedBuffer& buffer)
    : ResponseMessage(buffer) {
  parse_dynamic_headers(buffer.data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

SubscribeResponseMessage::SubscribeResponseMessage()
    : ResponseMessage(MessageType::SubscribeResponse) {}

const ParsedMessageValue& SubscribeResponseMessage::get_value() {
  if (_parsed_value == nullptr && body != nullptr) {
    _parsed_value.reset(new ParsedMessageValue(body->data(), body->size()));
  }
  return *_parsed_value;
}
void SubscribeResponseMessage::set_value(const Variant & value, const Variant & meta) {
  // TODO: create encode both meta and value, and create a Buffer Object to store the value
  // Buffer format  2bytes(meta size) + meta msgpack + value msgpack
}
}  // namespace dsa
