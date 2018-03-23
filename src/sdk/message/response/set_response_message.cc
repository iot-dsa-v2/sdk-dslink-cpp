#include "dsa_common.h"

#include "set_response_message.h"

namespace dsa {
SetResponseMessage::SetResponseMessage(const uint8_t* data, size_t size)
    : ResponseMessage(data, size) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
}

SetResponseMessage::SetResponseMessage()
    : ResponseMessage(MessageType::SET_RESPONSE) {}

const string_& SetResponseMessage::get_audit_log() const {
  return DynamicStringHeader::read_value(audit_log);
}
void SetResponseMessage::set_audit_log(const string_& value) {
  if (DynamicStringHeader::write_value(audit_log, DynamicHeader::AUDIT_LOG,
                                       value)) {
    static_headers.message_size = 0;
  }
}

const string_& SetResponseMessage::get_error_detail() const {
  return DynamicStringHeader::read_value(error_detail);
}
void SetResponseMessage::set_error_detail(const string_& value) {
  if (DynamicStringHeader::write_value(error_detail,
                                       DynamicHeader::ERROR_DETAIL, value)) {
    static_headers.message_size = 0;
  }
}

}  // namespace dsa
