#include "dsa_common.h"

#include "set_request_message.h"

namespace dsa {
SetRequestMessage::SetRequestMessage(const uint8_t* data, size_t size)
    : RequestMessage(data, size) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
  if (decode_all && get_page_id() == 0) {
    _cached_value.reset(new MessageValue(this));
  }
}

SetRequestMessage::SetRequestMessage()
    : RequestMessage(MessageType::SET_REQUEST) {}

SetRequestMessage::SetRequestMessage(const string_& path, Var&& value)
    : RequestMessage(MessageType::SET_REQUEST) {
  set_target_path(path);
  set_value(std::move(value));
}

MessageValue SetRequestMessage::get_value() const {
  if (_cached_value != nullptr) {
    MessageValue result = std::move(*_cached_value);
    _cached_value.reset();
    return std::move(result);
  }
  return MessageValue(this);
}
void SetRequestMessage::set_value(MessageValue&& value, int32_t sequence_id) {
  value.write(this, sequence_id);
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

void SetRequestMessage::print_body(std::ostream& os) const {
  if (body->size() > 0) {
    if (body->size() < 256) {
      MessageValue value = get_value();
      if (value.meta.is_map()) {
        os << " META: " << value.meta.to_json();
      }
      os << " VALUE: " << value.value.to_json();
    } else {
      os << " BODY SIZE: " << body->size();
    }
  }
}

}  // namespace dsa
