#include "list_response_message.h"

namespace dsa {
void ListResponseMessage::parse_dynamic_headers(const uint8_t* data, size_t size) {
  while (size > 0) {
    DynamicHeader* header = DynamicHeader::parse(data, size);
    uint8_t key = header->key();;
    if (key == DynamicHeader::Priority) {
      priority.reset(static_cast<DynamicByteHeader*>(header));
    } else if (key == DynamicHeader::Status) {
      status.reset(static_cast<DynamicByteHeader*>(header));
    } else if (key == DynamicHeader::SequenceId) {
      sequence_id.reset(static_cast<DynamicIntHeader*>(header));
    } else if (key == DynamicHeader::BasePath) {
      base_path.reset(static_cast<DynamicStringHeader*>(header));
    } else if (key == DynamicHeader::SourcePath) {
      source_path.reset(static_cast<DynamicStringHeader*>(header));
    }
  }
}

void ListResponseMessage::write_dynamic_data(uint8_t* data) const { 
  if (priority != nullptr) {
    priority->write(data);
    data += priority->size();
  } 
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
    memcpy(data, body->data, body->size);
  }
}
void ListResponseMessage::update_static_header() {
  uint32_t header_size = StaticHeaders::TotalSize; 
  if (priority != nullptr) {
    header_size += priority->size();
  } 
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
    message_size += body->size;
  }
  static_headers.message_size = message_size;
  static_headers.header_size = header_size;
}

} // namespace dsa
