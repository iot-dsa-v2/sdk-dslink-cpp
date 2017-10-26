#include "dsa_common.h"

#include "invoke_request_message.h"

namespace dsa {

InvokeRequestMessage::InvokeRequestMessage(const InvokeRequestMessage& from)
    : RequestMessage(from.static_headers) {
  if (from.priority != nullptr)
    priority.reset(new DynamicBoolHeader(DynamicHeader::PRIORITY));
  if (from.sequence_id != nullptr)
    sequence_id.reset(new DynamicIntHeader(DynamicHeader::SEQUENCE_ID, from.sequence_id->value()));
  if (from.page_id != nullptr)
    page_id.reset(new DynamicIntHeader(DynamicHeader::PAGE_ID, from.page_id->value()));
  if (from.alias_count != nullptr)
    alias_count.reset(new DynamicByteHeader(DynamicHeader::ALIAS_COUNT, from.alias_count->value()));
  if (from.target_path != nullptr)
    target_path.reset(new DynamicStringHeader(DynamicHeader::TARGET_PATH, from.target_path->value()));
  if (from.permission_token != nullptr)
    permission_token.reset(new DynamicStringHeader(DynamicHeader::PERMISSION_TOKEN, from.permission_token->value()));
  if (from.max_permission != nullptr)
    max_permission.reset(new DynamicByteHeader(DynamicHeader::MAX_PERMISSION, from.max_permission->value()));
  if (from.no_stream != nullptr)
    no_stream.reset(new DynamicBoolHeader(DynamicHeader::NO_STREAM));
  if (from.body != nullptr)
    body.reset(from.body.get());
}

void InvokeRequestMessage::parse_dynamic_data(const uint8_t *data, size_t dynamic_header_size, size_t body_size) throw(const MessageParsingError &) {
  while (dynamic_header_size > 0) {
    DynamicHeader *header = DynamicHeader::parse(data, dynamic_header_size);
    data += header->size();
    dynamic_header_size -= header->size();
    switch (header->key()) {
      case DynamicHeader::PRIORITY:priority.reset(DOWN_CAST<DynamicBoolHeader *>(header));
        break;
      case DynamicHeader::SEQUENCE_ID:sequence_id.reset(DOWN_CAST<DynamicIntHeader *>(header));
        break;
      case DynamicHeader::PAGE_ID:page_id.reset(DOWN_CAST<DynamicIntHeader *>(header));
        break;
      case DynamicHeader::ALIAS_COUNT:alias_count.reset(DOWN_CAST<DynamicByteHeader *>(header));
        break;
      case DynamicHeader::TARGET_PATH: {
        target_path.reset(DOWN_CAST<DynamicStringHeader *>(header));
        _parsed_target_path.reset(new Path(DOWN_CAST<DynamicStringHeader *>(header)->value()));
        break;
      }
      case DynamicHeader::PERMISSION_TOKEN:permission_token.reset(DOWN_CAST<DynamicStringHeader *>(header));
        break;
      case DynamicHeader::MAX_PERMISSION:max_permission.reset(DOWN_CAST<DynamicByteHeader *>(header));
        break;
      case DynamicHeader::NO_STREAM:no_stream.reset(DOWN_CAST<DynamicBoolHeader *>(header));
        break;
      default:throw MessageParsingError("Invalid dynamic header");
    }
  }
  if ( body_size > 0) {
      body.reset(new RefCountBytes(data, data + body_size));
  }
}

void InvokeRequestMessage::write_dynamic_data(uint8_t *data) const {
  if (priority != nullptr) {
    priority->write(data);
    data += priority->size();
  }
  if (sequence_id != nullptr) {
    sequence_id->write(data);
    data += sequence_id->size();
  }
  if (page_id != nullptr) {
    page_id->write(data);
    data += page_id->size();
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
  if (max_permission != nullptr) {
    max_permission->write(data);
    data += max_permission->size();
  }
  if (no_stream != nullptr) {
    no_stream->write(data);
    data += no_stream->size();
  }
  if (body != nullptr) {
    std::copy(body->begin(), body->end(), data);
  }
}

void InvokeRequestMessage::update_static_header() {
  uint32_t header_size = StaticHeaders::TOTAL_SIZE;
  if (priority != nullptr) {
    header_size += priority->size();
  }
  if (sequence_id != nullptr) {
    header_size += sequence_id->size();
  }
  if (page_id != nullptr) {
    header_size += page_id->size();
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
  if (max_permission != nullptr) {
    header_size += max_permission->size();
  }
  if (no_stream != nullptr) {
    header_size += no_stream->size();
  }

  uint32_t message_size = header_size; 
  if (body != nullptr) {
    message_size += body->size();
  }
  static_headers.message_size = message_size;
  static_headers.header_size = (uint16_t)header_size;
}

}  // namespace dsa
