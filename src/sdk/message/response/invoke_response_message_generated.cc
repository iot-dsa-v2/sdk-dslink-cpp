#include "dsa_common.h"

#include "invoke_response_message.h"

#include <iostream>

namespace dsa {

InvokeResponseMessage::InvokeResponseMessage(const InvokeResponseMessage& from)
    : ResponseMessage(from.static_headers) {
  if (from.status != nullptr)
    status.reset(new DynamicByteHeader(DynamicHeader::STATUS, from.status->value()));
  if (from.sequence_id != nullptr)
    sequence_id.reset(new DynamicIntHeader(DynamicHeader::SEQUENCE_ID, from.sequence_id->value()));
  if (from.page_id != nullptr)
    page_id.reset(new DynamicIntHeader(DynamicHeader::PAGE_ID, from.page_id->value()));
  if (from.refreshed != nullptr)
    refreshed.reset(new DynamicBoolHeader(DynamicHeader::REFRESHED));
  if (from.skippable != nullptr)
    skippable.reset(new DynamicBoolHeader(DynamicHeader::SKIPPABLE));
  if (from.body != nullptr)
    body.reset(from.body.get());
}

void InvokeResponseMessage::parse_dynamic_data(const uint8_t *data, size_t dynamic_header_size, size_t body_size) throw(const MessageParsingError &) {
  while (dynamic_header_size > 0) {
    auto header = DynamicHeader::parse(data, dynamic_header_size);
    data += header->size();
    dynamic_header_size -= header->size();
    switch (header->key()) {
      case DynamicHeader::STATUS:status.reset(DOWN_CAST<DynamicByteHeader *>(header.release()));
        break;
      case DynamicHeader::SEQUENCE_ID:sequence_id.reset(DOWN_CAST<DynamicIntHeader *>(header.release()));
        break;
      case DynamicHeader::PAGE_ID:page_id.reset(DOWN_CAST<DynamicIntHeader *>(header.release()));
        break;
      case DynamicHeader::REFRESHED:refreshed.reset(DOWN_CAST<DynamicBoolHeader *>(header.release()));
        break;
      case DynamicHeader::SKIPPABLE:skippable.reset(DOWN_CAST<DynamicBoolHeader *>(header.release()));
        break;
      default:throw MessageParsingError("Invalid dynamic header");
    }
  }
  if ( body_size > 0) {
      body.reset(new RefCountBytes(data, data + body_size));
  }
}

void InvokeResponseMessage::write_dynamic_data(uint8_t *data) const {
  if (status != nullptr) {
    status->write(data);
    data += status->size();
  }
  if (sequence_id != nullptr) {
    sequence_id->write(data);
    data += sequence_id->size();
  }
  if (page_id != nullptr) {
    page_id->write(data);
    data += page_id->size();
  }
  if (refreshed != nullptr) {
    refreshed->write(data);
    data += refreshed->size();
  }
  if (skippable != nullptr) {
    skippable->write(data);
    data += skippable->size();
  }
  if (body != nullptr) {
    std::copy(body->begin(), body->end(), data);
  }
}

void InvokeResponseMessage::update_static_header() {
  uint32_t header_size = StaticHeaders::TOTAL_SIZE;
  if (status != nullptr) {
    header_size += status->size();
  }
  if (sequence_id != nullptr) {
    header_size += sequence_id->size();
  }
  if (page_id != nullptr) {
    header_size += page_id->size();
  }
  if (refreshed != nullptr) {
    header_size += refreshed->size();
  }
  if (skippable != nullptr) {
    header_size += skippable->size();
  }

  uint32_t message_size = header_size; 
  if (body != nullptr) {
    message_size += body->size();
  }
  static_headers.message_size = message_size;
  static_headers.header_size = (uint16_t)header_size;
}

void InvokeResponseMessage::print_headers(std::ostream &os) const {

  if (status != nullptr) {
    os << " Status: x" << std::hex << int(status->value()) << std::dec;
  }
  if (sequence_id != nullptr) {
    os << " SequenceId: " << sequence_id->value();
  }
  if (page_id != nullptr) {
    os << " PageId: " << page_id->value();
  }
  if (refreshed != nullptr) {
    os << " Refreshed";
  }
  if (skippable != nullptr) {
    os << " Skippable";
  }
}

}  // namespace dsa
