#include "base_message.h"

namespace dsa {
Message::Message(const SharedBuffer& buffer) : static_headers(buffer.data){};

void Message::write(uint8_t* data) const throw(const std::runtime_error&) {
  if (!static_headers.message_size) {
    // message_size shouldn't be 0
    throw std::runtime_error("invalid message size");
  }
  static_headers.write(data);
  write_dynamic_data(data + StaticHeaders::TotalSize);
}

uint8_t Message::get_priority() const {
  return DynamicByteHeader::read_value(priority);
}
void Message::set_priority(uint8_t value) {
  if (DynamicByteHeader::write_value(priority, DynamicHeader::Priority,
                                     value)) {
    static_headers.message_size = 0;
  }
}

int32_t Message::get_sequence_id() const {
  return DynamicIntHeader::read_value(sequence_id);
}
void Message::set_sequence_id(int32_t value) {
  if (DynamicIntHeader::write_value(sequence_id, DynamicHeader::SequenceId,
                                    value)) {
    static_headers.message_size = 0;
  }
}

int32_t Message::get_page_id() const {
  return DynamicIntHeader::read_value(page_id);
}
void Message::set_page_id(int32_t value) {
  if (DynamicIntHeader::write_value(page_id, DynamicHeader::PageId, value)) {
    static_headers.message_size = 0;
  }
}

RequestMessage::RequestMessage(const SharedBuffer& buffer) : Message(buffer){};

const std::string& RequestMessage::get_target_path() const {
  return DynamicStringHeader::read_value(target_path);
}
void RequestMessage::set_target_path(const std::string& value) {
  if (DynamicStringHeader::write_value(target_path, DynamicHeader::TargetPath,
                                       value)) {
    static_headers.message_size = 0;
  }
}

const std::string& RequestMessage::get_permission_token() const {
  return DynamicStringHeader::read_value(permission_token);
}
void RequestMessage::set_permission_token(const std::string& value) {
  if (DynamicStringHeader::write_value(permission_token,
                                       DynamicHeader::PermissionToken, value)) {
    static_headers.message_size = 0;
  }
}

bool RequestMessage::get_no_stream() const {
  return DynamicBoolHeader::read_value(no_stream);
}
void RequestMessage::set_no_stream(bool value) {
  if (DynamicBoolHeader::write_value(no_stream, DynamicHeader::NoStream,
                                     value)) {
    static_headers.message_size = 0;
  }
}

uint8_t RequestMessage::get_alias_count() const {
  return DynamicByteHeader::read_value(alias_count);
}
void RequestMessage::set_alias_count(uint8_t value) {
  if (DynamicByteHeader::write_value(alias_count, DynamicHeader::AliasCount,
                                     value)) {
    static_headers.message_size = 0;
  }
}

ResponseMessage::ResponseMessage(const SharedBuffer& buffer)
    : Message(buffer){};

const std::string& ResponseMessage::get_source_path() const {
  return DynamicStringHeader::read_value(source_path);
}

void ResponseMessage::set_source_path(const std::string& value) {
  if (DynamicStringHeader::write_value(source_path, DynamicHeader::SourcePath, value)) {
    static_headers.message_size = 0;
  }
}

uint8_t ResponseMessage::get_status() const {
  return DynamicByteHeader::read_value(status);
}
void ResponseMessage::set_status(uint8_t value) {
  if (DynamicByteHeader::write_value(status, DynamicHeader::Status, value)) {
    static_headers.message_size = 0;
  }
}

}  // namespace dsa
