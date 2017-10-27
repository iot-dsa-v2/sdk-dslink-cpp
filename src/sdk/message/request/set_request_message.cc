#include "dsa_common.h"

#include "set_request_message.h"

namespace dsa {
SetRequestMessage::SetRequestMessage(const uint8_t* data, size_t size)
    : RequestMessage(data, size) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
}

SetRequestMessage::SetRequestMessage()
    : RequestMessage(MessageType::SET_REQUEST) {}

MessageValue SetRequestMessage::get_value() const {
  return MessageValue(body->data(), body->size());
}
void SetRequestMessage::set_value(MessageValue&& value) {
  body = value.to_msgpack();

  // invalidate message_size
  static_headers.message_size = 0;
}

const string_& SetRequestMessage::get_attribute_field() const {
  return DynamicStringHeader::read_value(attribute_field);
}
void SetRequestMessage::set_attribute_field(const string_& value) {
  if (DynamicStringHeader::write_value(attribute_field,
                                       DynamicHeader::ATTRIBUTE_FIELD, value)) {
    static_headers.message_size = 0;
  }
}

}  // namespace dsa
