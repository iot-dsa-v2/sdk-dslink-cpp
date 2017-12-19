#include "dsa_common.h"

#include "set_response_message.h"

#include <iostream>

namespace dsa {

SetResponseMessage::SetResponseMessage(const SetResponseMessage& from)
    : ResponseMessage(from.static_headers) {
  if (from.status != nullptr)
    status.reset(new DynamicByteHeader(DynamicHeader::STATUS, from.status->value()));
}

void SetResponseMessage::parse_dynamic_data(const uint8_t *data, size_t dynamic_header_size, size_t body_size) throw(const MessageParsingError &) {
  while (dynamic_header_size > 0) {
    auto header = DynamicHeader::parse(data, dynamic_header_size);
    data += header->size();
    dynamic_header_size -= header->size();
    switch (header->key()) {
      case DynamicHeader::STATUS:status.reset(DOWN_CAST<DynamicByteHeader *>(header.release()));
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
}

void SetResponseMessage::update_static_header() {
  uint32_t header_size = StaticHeaders::TOTAL_SIZE;
  if (status != nullptr) {
    header_size += status->size();
  }

  uint32_t message_size = header_size;
  static_headers.message_size = message_size;
  static_headers.header_size = (uint16_t)header_size;
}

void SetResponseMessage::print_headers(std::ostream &os) const {

  if (status != nullptr) {
    os << " Status:" << status->value();
  }
}

}  // namespace dsa
