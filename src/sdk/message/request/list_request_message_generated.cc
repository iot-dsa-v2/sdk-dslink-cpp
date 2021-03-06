#include "dsa_common.h"

#include "list_request_message.h"

#include <iostream>

namespace dsa {

ListRequestMessage::ListRequestMessage(const ListRequestMessage& from)
    : RequestMessage(from.static_headers) {
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
    auto header = DynamicHeader::parse(data, dynamic_header_size);
    data += header->size();
    dynamic_header_size -= header->size();
    switch (header->key()) {
      case DynamicHeader::ALIAS_COUNT:alias_count.reset(DOWN_CAST<DynamicByteHeader *>(header.release()));
        break;
      case DynamicHeader::TARGET_PATH: {
        target_path.reset(DOWN_CAST<DynamicStringHeader *>(header.release()));
        _parsed_target_path.reset(new Path(target_path->value()));
        break;
      }
      case DynamicHeader::PERMISSION_TOKEN:permission_token.reset(DOWN_CAST<DynamicStringHeader *>(header.release()));
        break;
      case DynamicHeader::NO_STREAM:no_stream.reset(DOWN_CAST<DynamicBoolHeader *>(header.release()));
        break;
      default:throw MessageParsingError("Invalid dynamic header");
    }
  }
}

void ListRequestMessage::write_dynamic_data(uint8_t *data) const {
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

void ListRequestMessage::print_headers(std::ostream &os) const {

  if (alias_count != nullptr) {
    os << " AliasCount: x" << std::hex << int(alias_count->value()) << std::dec;
  }
  if (target_path != nullptr) {
    os << " TargetPath: " << target_path->value();
  }
  if (permission_token != nullptr) {
    os << " PermissionToken: " << permission_token->value();
  }
  if (no_stream != nullptr) {
    os << " NoStream";
  }
}

}  // namespace dsa
