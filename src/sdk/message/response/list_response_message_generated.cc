#include "dsa_common.h"

#include "list_response_message.h"

namespace dsa {

ListResponseMessage::ListResponseMessage(const ListResponseMessage& from)
    : ResponseMessage(from.static_headers) {
  if (from.body != nullptr)
    body.reset(from.body.get());
  if (from.status != nullptr)
    status.reset(new DynamicByteHeader(DynamicHeader::STATUS, from.status->value()));
  if (from.sequence_id != nullptr)
    sequence_id.reset(new DynamicIntHeader(DynamicHeader::SEQUENCE_ID, from.sequence_id->value()));
  if (from.base_path != nullptr)
    base_path.reset(new DynamicStringHeader(DynamicHeader::BASE_PATH, from.base_path->value()));
  if (from.source_path != nullptr)
    source_path.reset(new DynamicStringHeader(DynamicHeader::SOURCE_PATH, from.source_path->value()));
}

void ListResponseMessage::parse_dynamic_headers(const uint8_t *data, size_t size) throw(const MessageParsingError &) {
  while (size > 0) {
    DynamicHeader *header = DynamicHeader::parse(data, size);
    data += header->size();
    size -= header->size();
    switch (header->key()) {
      case DynamicHeader::STATUS:status.reset(dynamic_cast<DynamicByteHeader *>(header));
        break;
      case DynamicHeader::SEQUENCE_ID:sequence_id.reset(dynamic_cast<DynamicIntHeader *>(header));
        break;
      case DynamicHeader::BASE_PATH:base_path.reset(dynamic_cast<DynamicStringHeader *>(header));
        break;
      case DynamicHeader::SOURCE_PATH:source_path.reset(dynamic_cast<DynamicStringHeader *>(header));
        break;
      default:throw MessageParsingError("Invalid dynamic header");
    }
  }
}

void ListResponseMessage::write_dynamic_data(uint8_t *data) const {
  if (status != nullptr) {
    status->write(data);
    data += status->size();
  }
  if (sequence_id != nullptr) {
    sequence_id->write(data);
    data += sequence_id->size();
  }
  if (base_path != nullptr) {
    base_path->write(data);
    data += base_path->size();
  }
  if (source_path != nullptr) {
    source_path->write(data);
    data += source_path->size();
  }
  if (body != nullptr) {
    std::copy(body->begin(), body->end(), data);
  }
}

void ListResponseMessage::update_static_header() {
  uint32_t header_size = StaticHeaders::TOTAL_SIZE;
  if (status != nullptr) {
    header_size += status->size();
  }
  if (sequence_id != nullptr) {
    header_size += sequence_id->size();
  }
  if (base_path != nullptr) {
    header_size += base_path->size();
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
