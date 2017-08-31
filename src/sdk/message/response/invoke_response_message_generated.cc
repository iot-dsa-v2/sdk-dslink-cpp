#include "dsa_common.h"

#include "invoke_response_message.h"

namespace dsa {

InvokeResponseMessage::InvokeResponseMessage(const InvokeResponseMessage& from)
    : ResponseMessage(from.static_headers) {
  if (from.body != nullptr)
    body.reset(from.body.get());
  if (from.status != nullptr)
    status.reset(new DynamicByteHeader(DynamicHeader::STATUS, from.status->value()));
  if (from.sequence_id != nullptr)
    sequence_id.reset(new DynamicIntHeader(DynamicHeader::SEQUENCE_ID, from.sequence_id->value()));
  if (from.page_id != nullptr)
    page_id.reset(new DynamicIntHeader(DynamicHeader::PAGE_ID, from.page_id->value()));
  if (from.skippable != nullptr)
    skippable.reset(new DynamicBoolHeader(DynamicHeader::SKIPPABLE));
}

void InvokeResponseMessage::parse_dynamic_headers(const uint8_t *data, size_t size) throw(const MessageParsingError &) {
  while (size > 0) {
    DynamicHeader *header = DynamicHeader::parse(data, size);
    data += header->size();
    size -= header->size();
    switch (header->key()) {
      case DynamicHeader::STATUS:status.reset(dynamic_cast<DynamicByteHeader *>(header));
        break;
      case DynamicHeader::SEQUENCE_ID:sequence_id.reset(dynamic_cast<DynamicIntHeader *>(header));
        break;
      case DynamicHeader::PAGE_ID:page_id.reset(dynamic_cast<DynamicIntHeader *>(header));
        break;
      case DynamicHeader::SKIPPABLE:skippable.reset(dynamic_cast<DynamicBoolHeader *>(header));
        break;
      default:throw MessageParsingError("Invalid dynamic header");
    }
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
  if (skippable != nullptr) {
    skippable->write(data);
    data += skippable->size();
  }
  if (body != nullptr) {
    std::copy(body->begin(), body->end(), data);
  }
}

void InvokeResponseMessage::update_static_header() {
  uint32_t header_size = StaticHeaders::TotalSize;
  if (status != nullptr) {
    header_size += status->size();
  }
  if (sequence_id != nullptr) {
    header_size += sequence_id->size();
  }
  if (page_id != nullptr) {
    header_size += page_id->size();
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

}  // namespace dsa
