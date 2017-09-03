#include "dsa_common.h"

#include "subscribe_response_message.h"

namespace dsa {

SubscribeResponseMessage::SubscribeResponseMessage(const SubscribeResponseMessage& from)
    : ResponseMessage(from.static_headers) {
  if (from.status != nullptr)
    status.reset(new DynamicByteHeader(DynamicHeader::STATUS, from.status->value()));
  if (from.sequence_id != nullptr)
    sequence_id.reset(new DynamicIntHeader(DynamicHeader::SEQUENCE_ID, from.sequence_id->value()));
  if (from.page_id != nullptr)
    page_id.reset(new DynamicIntHeader(DynamicHeader::PAGE_ID, from.page_id->value()));
  if (from.source_path != nullptr)
    source_path.reset(new DynamicStringHeader(DynamicHeader::SOURCE_PATH, from.source_path->value()));
  if (from.body != nullptr)
    body.reset(from.body.get());
}

void SubscribeResponseMessage::parse_dynamic_data(const uint8_t *data, size_t dynamic_header_size, size_t body_size) throw(const MessageParsingError &) {
  while (dynamic_header_size > 0) {
    DynamicHeader *header = DynamicHeader::parse(data, dynamic_header_size);
    data += header->size();
    dynamic_header_size -= header->size();
    switch (header->key()) {
      case DynamicHeader::STATUS:status.reset(dynamic_cast<DynamicByteHeader *>(header));
        break;
      case DynamicHeader::SEQUENCE_ID:sequence_id.reset(dynamic_cast<DynamicIntHeader *>(header));
        break;
      case DynamicHeader::PAGE_ID:page_id.reset(dynamic_cast<DynamicIntHeader *>(header));
        break;
      case DynamicHeader::SOURCE_PATH:source_path.reset(dynamic_cast<DynamicStringHeader *>(header));
        break;
      default:throw MessageParsingError("Invalid dynamic header");
    }
  }
  if ( body_size > 0) {
      body.reset(new IntrusiveBytes(data, data + body_size));
  }
}

void SubscribeResponseMessage::write_dynamic_data(uint8_t *data) const {
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
  if (source_path != nullptr) {
    source_path->write(data);
    data += source_path->size();
  }
  if (body != nullptr) {
    std::copy(body->begin(), body->end(), data);
  }
}

void SubscribeResponseMessage::update_static_header() {
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
  if (source_path != nullptr) {
    header_size += source_path->size();
  }

  uint32_t message_size = header_size; 
  if (body != nullptr) {
    message_size += body->size();
  }
  static_headers.message_size = message_size;
  static_headers.header_size = (uint16_t)header_size;
}

}  // namespace dsa
