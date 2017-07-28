#include "dsa_common.h"

#include "set_response_message.h"

namespace dsa {

SetResponseMessage::SetResponseMessage(const SetResponseMessage& from)
    : ResponseMessage(from.static_headers) { 
  if (from.priority != nullptr) {
     priority.reset(new DynamicBoolHeader(DynamicHeader::Priority));
  } 
  if (from.status != nullptr) {
    status.reset(new DynamicByteHeader(DynamicHeader::Status, from.status->value()));
  }
}

void SetResponseMessage::parse_dynamic_headers(const uint8_t* data, size_t size) {
  while (size > 0) {
    DynamicHeader* header = DynamicHeader::parse(data, size);
    data += header->size();
    size -= header->size();
    uint8_t key = header->key();;
    if (key == DynamicHeader::Priority) {
      priority.reset(static_cast<DynamicBoolHeader*>(header));
    } else if (key == DynamicHeader::Status) {
      status.reset(static_cast<DynamicByteHeader*>(header));
    }
  }
}

void SetResponseMessage::write_dynamic_data(uint8_t* data) const { 
  if (priority != nullptr) {
    priority->write(data);
    data += priority->size();
  } 
  if (status != nullptr) {
    status->write(data);
    data += status->size();
  }
}

void SetResponseMessage::update_static_header() {
  uint32_t header_size = StaticHeaders::TotalSize; 
  if (priority != nullptr) {
    header_size += priority->size();
  } 
  if (status != nullptr) {
    header_size += status->size();
  }

  uint32_t message_size = header_size;
  static_headers.message_size = message_size;
  static_headers.header_size = header_size;
}

} // namespace dsa
