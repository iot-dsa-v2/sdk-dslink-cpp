#include "dsa_common.h"

#include "invoke_response_message.h"

#include "variant/variant.h"

namespace dsa {
InvokeResponseMessage::InvokeResponseMessage(const uint8_t* data, size_t size)
    : ResponseMessage(data, size) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
}

InvokeResponseMessage::InvokeResponseMessage()
    : ResponseMessage(MessageType::INVOKE_RESPONSE) {}

void InvokeResponseMessage::set_value(const Var& value) {
  set_body(new RefCountBytes(value.to_msgpack()));
}
Var InvokeResponseMessage::get_value() const {
  if (body != nullptr && !body->empty()) {
    return Var::from_msgpack(body->data(), body->size());
  }
  return Var();
}

const bool InvokeResponseMessage::get_skippable() const {
  return DynamicBoolHeader::read_value(skippable);
}
void InvokeResponseMessage::set_skippable(bool value) {
  if (DynamicBoolHeader::write_value(skippable, DynamicHeader::SKIPPABLE,
                                     value)) {
    static_headers.message_size = 0;
  }
}

const bool InvokeResponseMessage::get_refreshed() const {
  return DynamicBoolHeader::read_value(refreshed);
}
void InvokeResponseMessage::set_refreshed(bool value) {
  if (DynamicBoolHeader::write_value(refreshed, DynamicHeader::REFRESHED,
                                     value)) {
    static_headers.message_size = 0;
  }
}

}  // namespace dsa
