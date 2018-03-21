#include "dsa_common.h"

#include "set_response_message.h"

#include <iostream>

namespace dsa {

SetResponseMessage::SetResponseMessage(const SetResponseMessage& from)
    : ResponseMessage(from.static_headers) {
  if (from.status != nullptr)
    status.reset(new DynamicByteHeader(DynamicHeader::STATUS, from.status->value()));
  if (from.audit_log != nullptr)
    audit_log.reset(new DynamicStringHeader(DynamicHeader::AUDIT_LOG, from.audit_log->value()));
  if (from.error_detail != nullptr)
    error_detail.reset(new DynamicStringHeader(DynamicHeader::ERROR_DETAIL, from.error_detail->value()));
}

void SetResponseMessage::parse_dynamic_data(const uint8_t *data, size_t dynamic_header_size, size_t body_size) throw(const MessageParsingError &) {
  while (dynamic_header_size > 0) {
    auto header = DynamicHeader::parse(data, dynamic_header_size);
    data += header->size();
    dynamic_header_size -= header->size();
    switch (header->key()) {
      case DynamicHeader::STATUS:status.reset(DOWN_CAST<DynamicByteHeader *>(header.release()));
        break;
      case DynamicHeader::AUDIT_LOG:audit_log.reset(DOWN_CAST<DynamicStringHeader *>(header.release()));
        break;
      case DynamicHeader::ERROR_DETAIL:error_detail.reset(DOWN_CAST<DynamicStringHeader *>(header.release()));
        break;
      default:throw MessageParsingError("Invalid dynamic header");
    }
  }
}

void SetResponseMessage::write_dynamic_data(uint8_t *data) const {
  if (status != nullptr) {
    status->write(data);
    data += status->size();
  }
  if (audit_log != nullptr) {
    audit_log->write(data);
    data += audit_log->size();
  }
  if (error_detail != nullptr) {
    error_detail->write(data);
    data += error_detail->size();
  }
}

void SetResponseMessage::update_static_header() {
  uint32_t header_size = StaticHeaders::TOTAL_SIZE;
  if (status != nullptr) {
    header_size += status->size();
  }
  if (audit_log != nullptr) {
    header_size += audit_log->size();
  }
  if (error_detail != nullptr) {
    header_size += error_detail->size();
  }

  uint32_t message_size = header_size;
  static_headers.message_size = message_size;
  static_headers.header_size = (uint16_t)header_size;
}

void SetResponseMessage::print_headers(std::ostream &os) const {

  if (status != nullptr) {
    os << " Status: x" << std::hex << int(status->value()) << std::dec;
  }
  if (audit_log != nullptr) {
    os << " AuditLog: " << audit_log->value();
  }
  if (error_detail != nullptr) {
    os << " ErrorDetail: " << error_detail->value();
  }
}

}  // namespace dsa
