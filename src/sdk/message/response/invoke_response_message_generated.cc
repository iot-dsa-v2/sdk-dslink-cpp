#include "invoke_response_message.h"

#include "dsa_common.h"

namespace dsa {

InvokeResponseMessage::InvokeResponseMessage(const InvokeResponseMessage& from)
    : ResponseMessage(from.static_headers) {
  if (from.body != nullptr)
    body.reset(new SharedBuffer(*from.body));
  if (from.status != nullptr)
    status.reset(new DynamicByteHeader(DynamicHeader::Status, from.status->value()));
  if (from.sequence_id != nullptr)
    sequence_id.reset(new DynamicIntHeader(DynamicHeader::SequenceId, from.sequence_id->value()));
  if (from.page_id != nullptr)
    page_id.reset(new DynamicIntHeader(DynamicHeader::PageId, from.page_id->value()));
  if (from.skippable != nullptr)
    skippable.reset(new DynamicBoolHeader(DynamicHeader::Skippable));
}

void InvokeResponseMessage::parse_dynamic_headers(const uint8_t *data, size_t size) throw(const MessageParsingError &) {
  while (size > 0) {
    DynamicHeader *header = DynamicHeader::parse(data, size);
    data += header->size();
    size -= header->size();
    switch (header->key()) {
      case DynamicHeader::Status:status.reset(dynamic_cast<DynamicByteHeader *>(header));
        break;
      case DynamicHeader::SequenceId:sequence_id.reset(dynamic_cast<DynamicIntHeader *>(header));
        break;
      case DynamicHeader::PageId:page_id.reset(dynamic_cast<DynamicIntHeader *>(header));
        break;
      case DynamicHeader::Skippable:skippable.reset(dynamic_cast<DynamicBoolHeader *>(header));
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
    memcpy(data, body->data, body->size);
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
    message_size += body->size;
  }
  static_headers.message_size = message_size;
  static_headers.header_size = header_size;
}

}  // namespace dsa
