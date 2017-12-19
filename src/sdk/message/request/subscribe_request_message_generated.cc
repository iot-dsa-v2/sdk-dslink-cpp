#include "dsa_common.h"

#include "subscribe_request_message.h"

#include <iostream>

namespace dsa {

SubscribeRequestMessage::SubscribeRequestMessage(const SubscribeRequestMessage& from)
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
  if (from.qos != nullptr)
    qos.reset(new DynamicByteHeader(DynamicHeader::QOS, from.qos->value()));
  if (from.queue_size != nullptr)
    queue_size.reset(new DynamicIntHeader(DynamicHeader::QUEUE_SIZE, from.queue_size->value()));
  if (from.queue_time != nullptr)
    queue_time.reset(new DynamicIntHeader(DynamicHeader::QUEUE_TIME, from.queue_time->value()));
}

void SubscribeRequestMessage::parse_dynamic_data(const uint8_t *data, size_t dynamic_header_size, size_t body_size) throw(const MessageParsingError &) {
  while (dynamic_header_size > 0) {
    auto header = DynamicHeader::parse(data, dynamic_header_size);
    data += header->size();
    dynamic_header_size -= header->size();
    switch (header->key()) {
      case DynamicHeader::PRIORITY:priority.reset(DOWN_CAST<DynamicBoolHeader *>(header.release()));
        break;
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
      case DynamicHeader::QOS:qos.reset(DOWN_CAST<DynamicByteHeader *>(header.release()));
        break;
      case DynamicHeader::QUEUE_SIZE:queue_size.reset(DOWN_CAST<DynamicIntHeader *>(header.release()));
        break;
      case DynamicHeader::QUEUE_TIME:queue_time.reset(DOWN_CAST<DynamicIntHeader *>(header.release()));
        break;
      default:throw MessageParsingError("Invalid dynamic header");
    }
  }
}

void SubscribeRequestMessage::write_dynamic_data(uint8_t *data) const {
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
  if (qos != nullptr) {
    qos->write(data);
    data += qos->size();
  }
  if (queue_size != nullptr) {
    queue_size->write(data);
    data += queue_size->size();
  }
  if (queue_time != nullptr) {
    queue_time->write(data);
    data += queue_time->size();
  }
}

void SubscribeRequestMessage::update_static_header() {
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
  if (qos != nullptr) {
    header_size += qos->size();
  }
  if (queue_size != nullptr) {
    header_size += queue_size->size();
  }
  if (queue_time != nullptr) {
    header_size += queue_time->size();
  }

  uint32_t message_size = header_size;
  static_headers.message_size = message_size;
  static_headers.header_size = (uint16_t)header_size;
}

void SubscribeRequestMessage::print_headers(std::ostream &os) const {

  if (priority != nullptr) {
    os << " Priority";
  }
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
  if (qos != nullptr) {
    os << " Qos: x" << std::hex << int(qos->value()) << std::dec;
  }
  if (queue_size != nullptr) {
    os << " QueueSize: " << queue_size->value();
  }
  if (queue_time != nullptr) {
    os << " QueueTime: " << queue_time->value();
  }
}

}  // namespace dsa
