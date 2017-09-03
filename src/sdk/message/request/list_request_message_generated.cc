#include "dsa_common.h"

#include "list_request_message.h"

namespace dsa {

ListRequestMessage::ListRequestMessage(const ListRequestMessage& from)
    : RequestMessage(from.static_headers) {
  if (from.priority != nullptr)
    priority.reset(new DynamicBoolHeader(DynamicHeader::PRIORITY));
  if (from.alias_count != nullptr)
    alias_count.reset(new DynamicByteHeader(DynamicHeader::ALIAS_COUNT, from.alias_count->value()));
  if (from.target_path != nullptr)
    target_path.reset(new DynamicStringHeader(DynamicHeader::TARGET_PATH, from.target_path->value()));
  if (from.permission_token != nullptr)
    permission_token.reset(new DynamicStringHeader(DynamicHeader::PERMISSION_TOKEN, from.permission_token->value()));
  if (from.no_stream != nullptr)
    no_stream.reset(new DynamicBoolHeader(DynamicHeader::NO_STREAM));
}

void ListRequestMessage::parse_dynamic_data(const uint8_t *data, size_t dynamic_header_size, size_t body_size) throw(const MessageParsingError &) {
  while (dynamic_header_size > 0) {
    DynamicHeader *header = DynamicHeader::parse(data, dynamic_header_size);
    data += header->size();
    dynamic_header_size -= header->size();
    switch (header->key()) {
      case DynamicHeader::PRIORITY:priority.reset(dynamic_cast<DynamicBoolHeader *>(header));
        break;
      case DynamicHeader::ALIAS_COUNT:alias_count.reset(dynamic_cast<DynamicByteHeader *>(header));
        break;
      case DynamicHeader::TARGET_PATH:target_path.reset(dynamic_cast<DynamicStringHeader *>(header));
        break;
      case DynamicHeader::PERMISSION_TOKEN:permission_token.reset(dynamic_cast<DynamicStringHeader *>(header));
        break;
      case DynamicHeader::NO_STREAM:no_stream.reset(dynamic_cast<DynamicBoolHeader *>(header));
        break;
      default:throw MessageParsingError("Invalid dynamic header");
    }
  }
}

void ListRequestMessage::write_dynamic_data(uint8_t *data) const {
  if (priority != nullptr) {
    priority->write(data);
    data += priority->size();
  }
  if (alias_count != nullptr) {
    alias_count->write(data);
    data += alias_count->size();
  }
  if (target_path != nullptr) {
    target_path->write(data);
    data += target_path->size();
  }
  if (permission_token != nullptr) {
    permission_token->write(data);
    data += permission_token->size();
  }
  if (no_stream != nullptr) {
    no_stream->write(data);
    data += no_stream->size();
  }
}

void ListRequestMessage::update_static_header() {
  uint32_t header_size = StaticHeaders::TOTAL_SIZE;
  if (priority != nullptr) {
    header_size += priority->size();
  }
  if (alias_count != nullptr) {
    header_size += alias_count->size();
  }
  if (target_path != nullptr) {
    header_size += target_path->size();
  }
  if (permission_token != nullptr) {
    header_size += permission_token->size();
  }
  if (no_stream != nullptr) {
    header_size += no_stream->size();
  }

  uint32_t message_size = header_size;
  static_headers.message_size = message_size;
  static_headers.header_size = (uint16_t)header_size;
}

}  // namespace dsa
