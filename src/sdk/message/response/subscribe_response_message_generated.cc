#include "dsa_common.h"

#include "subscribe_response_message.h"

namespace dsa {

SubscribeResponseMessage::SubscribeResponseMessage(const SubscribeResponseMessage& from)
    : ResponseMessage(from.static_headers) { 
  if (from.status != nullptr) {
    status.reset(new DynamicByteHeader(DynamicHeader::Status, from.status->value()));
  } 
  if (from.sequence_id != nullptr) {
    sequence_id.reset(new DynamicIntHeader(DynamicHeader::SequenceId, from.sequence_id->value()));
  } 
  if (from.page_id != nullptr) {
    page_id.reset(new DynamicIntHeader(DynamicHeader::PageId, from.page_id->value()));
  } 
  if (from.source_path != nullptr) {
    source_path.reset(new DynamicStringHeader(DynamicHeader::SourcePath, from.source_path->value()));
  } 
  if (from.body != nullptr) {
    body.reset(new SharedBuffer(*from.body));
  }
}

void SubscribeResponseMessage::parse_dynamic_headers(const uint8_t* data, size_t size) {
  while (size > 0) {
    DynamicHeader* header = DynamicHeader::parse(data, size);
    data += header->size();
    size -= header->size();
    uint8_t key = header->key();
    if (key == DynamicHeader::Status) {
      status.reset(static_cast<DynamicByteHeader*>(header));
    } else if (key == DynamicHeader::SequenceId) {
      sequence_id.reset(static_cast<DynamicIntHeader*>(header));
    } else if (key == DynamicHeader::PageId) {
      page_id.reset(static_cast<DynamicIntHeader*>(header));
    } else if (key == DynamicHeader::SourcePath) {
      source_path.reset(static_cast<DynamicStringHeader*>(header));
    }
  }
}

void SubscribeResponseMessage::write_dynamic_data(uint8_t* data) const { 
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
    memcpy(data, body->data, body->size);
  }
}

void SubscribeResponseMessage::update_static_header() {
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
