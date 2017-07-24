#include "subscribe_response_message.h"

namespace dsa {
SubscribeResponseMessage::SubscribeResponseMessage(const SharedBuffer& buffer)
    : ResponseMessage(buffer) {
  parse_dynamic_headers(buffer.data + StaticHeaders::TotalSize,
                      static_headers.header_size - StaticHeaders::TotalSize);
}

void SubscribeResponseMessage::parse_dynamic_headers(const uint8_t* data,
                                                   size_t size) {
  while (size > 0) {
    DynamicHeader* header = DynamicHeader::parse(data, size);
    uint8_t key = header->key();

    if (key == DynamicHeader::Priority) {
      priority.reset(static_cast<DynamicByteHeader*>(header));
    }  //
    else if (key == DynamicHeader::Status) {
      status.reset(static_cast<DynamicByteHeader*>(header));
    }  //
    else if (key == DynamicHeader::SequenceId) {
      sequence_id.reset(static_cast<DynamicIntHeader*>(header));
    }  //
    else if (key == DynamicHeader::PageId) {
      page_id.reset(static_cast<DynamicIntHeader*>(header));
    }  //
    // else if (key == DynamicHeader::AliasCount) {
    //  alias_count.reset(static_cast<DynamicByteHeader*>(header));
    //} //
    // else if (key == DynamicHeader::TargetPath) {
    //  target_path.reset(static_cast<DynamicStringHeader*>(header));
    //}  //
    // else if (key == DynamicHeader::PermissionToken) {
    //  permission_token.reset(static_cast<DynamicStringHeader*>(header));
    //}  //
    // else if (key == DynamicHeader::MaxPermission) {
    //  max_permission_token.reset(static_cast<DynamicByteHeader*>(header));
    //} //
    // else if (key == DynamicHeader::NoStream) {
    //  no_stream.reset(static_cast<DynamicBoolHeader*>(header));
    //}  //
    // else if (key == DynamicHeader::Qos) {
    //  qos.reset(static_cast<DynamicByteHeader*>(header));
    //}  //
    // else if (key == DynamicHeader::QueueSize) {
    //  queue_size.reset(static_cast<DynamicByteHeader*>(header));
    //}  //
    // else if (key == DynamicHeader::QueueTime) {
    //  queue_time.reset(static_cast<DynamicByteHeader*>(header));
    //}  //
    // else if (key == DynamicHeader::UpdateFrequency) {
    //  update_frequency.reset(static_cast<DynamicByteHeader*>(header));
    //}  //
    // else if (key == DynamicHeader::BasePath) {
    //  base_path.reset(static_cast<DynamicStringHeader*>(header));
    //} //
    else if (key == DynamicHeader::SourcePath) {
      source_path.reset(static_cast<DynamicStringHeader*>(header));
    }  //
    // else if (key == DynamicHeader::Skippable) {
    //  skippable.reset(static_cast<DynamicBoolHeader*>(header));
    //} //
  }
}

void SubscribeResponseMessage::write_dynamic_data(uint8_t* data) const {
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
  if (page_id != nullptr) {
    page_id->write(data);
    data += page_id->size();
  }
  //if (alias_count != nullptr) {
  //  alias_count->write(data);
  //  data += alias_count->size();
  //}
  //if (permission_token != nullptr) {
  //  permission_token->write(data);
  //  data += permission_token->size();
  //}
  //if (max_permission != nullptr) {
  //  max_permission->write(data);
  //  data += max_permission->size();
  //}
  //if (no_stream != nullptr) {
  //  no_stream->write(data);
  //  data += no_stream->size();
  //}
  //if (qos != nullptr) {
  //  qos->write(data);
  //  data += qos->size();
  //}
  //if (queue_size != nullptr) {
  //  queue_size->write(data);
  //  data += queue_size->size();
  //}
  //if (queue_time != nullptr) {
  //  queue_time->write(data);
  //  data += queue_time->size();
  //}
  //if (update_frequency != nullptr) {
  //  update_frequency->write(data);
  //  data += update_frequency->size();
  //}
  //if (base_path != nullptr) {
  //  base_path->write(data);
  //  data += base_path->size();
  //}
  if (source_path != nullptr) {
    source_path->write(data);
    data += source_path->size();
  }
  //if (skippable != nullptr) {
  //  skippable->write(data);
  //  data += skippable->size();
  //}

  if (body != nullptr) {
    memcpy(data, body->data, body->size);
  }
}
void SubscribeResponseMessage::update_static_header() {
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
  if (page_id != nullptr) {
    header_size += page_id->size();
  }
  //if (alias_count != nullptr) {
  //  header_size += alias_count->size();
  //}
  //if (target_path != nullptr) {
  //  header_size += target_path->size();
  //}
  //if (permission_token != nullptr) {
  //  header_size += permission_token->size();
  //}
  //if (max_permission != nullptr) {
  //  header_size += max_permission->size();
  //}
  //if (no_stream != nullptr) {
  //  header_size += no_stream->size();
  //}
  //if (qos != nullptr) {
  //  header_size += qos->size();
  //}
  //if (queue_size != nullptr) {
  //  header_size += queue_size->size();
  //}
  //if (queue_time != nullptr) {
  //  header_size += queue_time->size();
  //}
  //if (update_frequency != nullptr) {
  //  header_size += update_frequency->size();
  //}
  //if (base_path != nullptr) {
  //  header_size += base_path->size();
  //}
  if (source_path != nullptr) {
    header_size += source_path->size();
  }
  //if (skippable != nullptr) {
  //  header_size += skippable->size();
  //}

  uint32_t message_size = header_size;
  if (body != nullptr) {
    message_size += body->size;
  }
  static_headers.message_size = message_size;
  static_headers.header_size = header_size;
}

}  // namespace dsa
