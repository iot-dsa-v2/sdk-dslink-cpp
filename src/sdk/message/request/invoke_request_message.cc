#include "dsa_common.h"

#include "invoke_request_message.h"

#include "variant/variant.h"

namespace dsa {

InvokeRequestMessage::InvokeRequestMessage(const uint8_t* data, size_t size)
    : RequestMessage(data, size) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
}

InvokeRequestMessage::InvokeRequestMessage()
    : RequestMessage(MessageType::INVOKE_REQUEST) {}

void InvokeRequestMessage::set_value(const Var& value) {
  set_body(new RefCountBytes(value.to_msgpack()));
}
Var InvokeRequestMessage::get_value() const {
  if (body != nullptr && !body->empty()) {
    return Var::from_msgpack(body->data(), body->size());
  }
  return Var();
}

const bool InvokeRequestMessage::get_skippable() const {
  return DynamicBoolHeader::read_value(skippable);
}
void InvokeRequestMessage::set_skippable(bool value) {
  if (DynamicBoolHeader::write_value(skippable, DynamicHeader::SKIPPABLE,
                                     value)) {
    static_headers.message_size = 0;
  }
}

const bool InvokeRequestMessage::get_refreshed() const {
  return DynamicBoolHeader::read_value(refreshed);
}
void InvokeRequestMessage::set_refreshed(bool value) {
  if (DynamicBoolHeader::write_value(refreshed, DynamicHeader::REFRESHED,
                                     value)) {
    static_headers.message_size = 0;
  }
}

}  // namespace dsa
