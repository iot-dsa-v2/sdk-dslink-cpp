#include "dsa_common.h"

#include "subscribe_request_message.h"

namespace dsa {

SubscribeRequestMessage::SubscribeRequestMessage(const SubscribeRequestMessage& from)
    : RequestMessage(from.static_headers) { 
  if (from.priority != nullptr) {
     priority.reset(new DynamicBoolHeader(DynamicHeader::Priority));
  } 
  if (from.alias_count != nullptr) {
    alias_count.reset(new DynamicByteHeader(DynamicHeader::AliasCount, from.alias_count->value()));
  } 
  if (from.permission_token != nullptr) {
    permission_token.reset(new DynamicStringHeader(DynamicHeader::PermissionToken, from.permission_token->value()));
  } 
  if (from.target_path != nullptr) {
    target_path.reset(new DynamicStringHeader(DynamicHeader::TargetPath, from.target_path->value()));
  } 
  if (from.no_stream != nullptr) {
    no_stream.reset(new DynamicBoolHeader(DynamicHeader::NoStream));
  } 
  if (from.qos != nullptr) {
    qos.reset(new DynamicByteHeader(DynamicHeader::Qos, from.qos->value()));
  } 
  if (from.queue_size != nullptr) {
    queue_size.reset(new DynamicIntHeader(DynamicHeader::QueueSize, from.queue_size->value()));
  } 
  if (from.queue_time != nullptr) {
    queue_time.reset(new DynamicIntHeader(DynamicHeader::QueueTime, from.queue_time->value()));
  }
}

void SubscribeRequestMessage::parse_dynamic_headers(const uint8_t* data, size_t size) {
  while (size > 0) {
    DynamicHeader* header = DynamicHeader::parse(data, size);
    data += header->size();
    size -= header->size();
    uint8_t key = header->key();;
    if (key == DynamicHeader::Priority) {
      priority.reset(static_cast<DynamicBoolHeader*>(header));
    } else if (key == DynamicHeader::AliasCount) {
      alias_count.reset(static_cast<DynamicByteHeader*>(header));
    } else if (key == DynamicHeader::TargetPath) {
      target_path.reset(static_cast<DynamicStringHeader*>(header));
    } else if (key == DynamicHeader::PermissionToken) {
      permission_token.reset(static_cast<DynamicStringHeader*>(header));
    } else if (key == DynamicHeader::NoStream) {
      no_stream.reset(static_cast<DynamicBoolHeader*>(header));
    } else if (key == DynamicHeader::Qos) {
      qos.reset(static_cast<DynamicByteHeader*>(header));
    } else if (key == DynamicHeader::QueueSize) {
      queue_size.reset(static_cast<DynamicIntHeader*>(header));
    } else if (key == DynamicHeader::QueueTime) {
      queue_time.reset(static_cast<DynamicIntHeader*>(header));
    }
  }
}

void SubscribeRequestMessage::write_dynamic_data(uint8_t* data) const { 
  if (priority != nullptr) {
    priority->write(data);
    data += priority->size();
  } 
  if (alias_count != nullptr) {
    alias_count->write(data);
    data += alias_count->size();
  } 
  if (permission_token != nullptr) {
    permission_token->write(data);
    data += permission_token->size();
  } 
  if (target_path != nullptr) {
    target_path->write(data);
    data += target_path->size();
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
  uint32_t header_size = StaticHeaders::TotalSize; 
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
  static_headers.header_size = header_size;
}

} // namespace dsa
