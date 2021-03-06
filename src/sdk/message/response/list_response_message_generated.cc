#include "dsa_common.h"

#include "list_response_message.h"

#include <iostream>

namespace dsa {

ListResponseMessage::ListResponseMessage(const ListResponseMessage& from)
    : ResponseMessage(from.static_headers) {
  if (from.status != nullptr)
    status.reset(new DynamicByteHeader(DynamicHeader::STATUS, from.status->value()));
  if (from.refreshed != nullptr)
    refreshed.reset(new DynamicBoolHeader(DynamicHeader::REFRESHED));
  if (from.sequence_id != nullptr)
    sequence_id.reset(new DynamicIntHeader(DynamicHeader::SEQUENCE_ID, from.sequence_id->value()));
  if (from.pub_path != nullptr)
    pub_path.reset(new DynamicStringHeader(DynamicHeader::PUB_PATH, from.pub_path->value()));
  if (from.source_path != nullptr)
    source_path.reset(new DynamicStringHeader(DynamicHeader::SOURCE_PATH, from.source_path->value()));
  if (from.body != nullptr)
    body.reset(from.body.get());
}

void ListResponseMessage::parse_dynamic_data(const uint8_t *data, size_t dynamic_header_size, size_t body_size) throw(const MessageParsingError &) {
  while (dynamic_header_size > 0) {
    auto header = DynamicHeader::parse(data, dynamic_header_size);
    data += header->size();
    dynamic_header_size -= header->size();
    switch (header->key()) {
      case DynamicHeader::STATUS:status.reset(DOWN_CAST<DynamicByteHeader *>(header.release()));
        break;
      case DynamicHeader::REFRESHED:refreshed.reset(DOWN_CAST<DynamicBoolHeader *>(header.release()));
        break;
      case DynamicHeader::SEQUENCE_ID:sequence_id.reset(DOWN_CAST<DynamicIntHeader *>(header.release()));
        break;
      case DynamicHeader::PUB_PATH:pub_path.reset(DOWN_CAST<DynamicStringHeader *>(header.release()));
        break;
      case DynamicHeader::SOURCE_PATH:source_path.reset(DOWN_CAST<DynamicStringHeader *>(header.release()));
        break;
      default:throw MessageParsingError("Invalid dynamic header");
    }
  }
  if ( body_size > 0) {
      body.reset(new RefCountBytes(data, data + body_size));
      parse_map_to(_raw_map); // parse the map right after decoding
  }
}

void ListResponseMessage::write_dynamic_data(uint8_t *data) const {
  if (status != nullptr) {
    status->write(data);
    data += status->size();
  }
  if (refreshed != nullptr) {
    refreshed->write(data);
    data += refreshed->size();
  }
  if (sequence_id != nullptr) {
    sequence_id->write(data);
    data += sequence_id->size();
  }
  if (pub_path != nullptr) {
    pub_path->write(data);
    data += pub_path->size();
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
  if (refreshed != nullptr) {
    header_size += refreshed->size();
  }
  if (sequence_id != nullptr) {
    header_size += sequence_id->size();
  }
  if (pub_path != nullptr) {
    header_size += pub_path->size();
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

void ListResponseMessage::print_headers(std::ostream &os) const {

  if (status != nullptr) {
    os << " Status: x" << std::hex << int(status->value()) << std::dec;
  }
  if (refreshed != nullptr) {
    os << " Refreshed";
  }
  if (sequence_id != nullptr) {
    os << " SequenceId: " << sequence_id->value();
  }
  if (pub_path != nullptr) {
    os << " PubPath: " << pub_path->value();
  }
  if (source_path != nullptr) {
    os << " SourcePath: " << source_path->value();
  }
}

}  // namespace dsa
