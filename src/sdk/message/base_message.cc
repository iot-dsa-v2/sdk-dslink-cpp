#include "dsa_common.h"

#include "base_message.h"

namespace dsa {
Message::Message(const uint8_t* data, size_t size) : static_headers(data){};
Message::Message(MessageType type) : static_headers(0, 0, type, 0, 0){};
Message::Message(const StaticHeaders& headers) : static_headers(headers){};

uint32_t Message::size() const {
  if (!static_headers.message_size) {
    const_cast<Message*>(this)->update_static_header();
  }
  return static_headers.message_size;
}
void Message::write(uint8_t* data) const throw(const MessageParsingError&) {
  if (static_headers.message_size == 0) {
    // message_size shouldn't be 0
    throw MessageParsingError("invalid message size");
  }
  static_headers.write(data);
  write_dynamic_data(data + StaticHeaders::TotalSize);
}

int32_t Message::get_sequence_id() const {
  return DynamicIntHeader::read_value(sequence_id);
}
void Message::set_sequence_id(int32_t value) {
  if (DynamicIntHeader::write_value(sequence_id, DynamicHeader::SEQUENCE_ID,
                                    value)) {
    static_headers.message_size = 0;
  }
}

int32_t Message::get_page_id() const {
  return DynamicIntHeader::read_value(page_id);
}
void Message::set_page_id(int32_t value) {
  if (DynamicIntHeader::write_value(page_id, DynamicHeader::PAGE_ID, value)) {
    static_headers.message_size = 0;
  }
}

PagedMessageMixin::PagedMessageMixin() : current_page(SIZE_MAX) {}

RequestMessage::RequestMessage(const uint8_t* data, size_t size) : Message(data, size){};
RequestMessage::RequestMessage(MessageType type) : Message(type){};
RequestMessage::RequestMessage(const StaticHeaders& headers)
    : Message(headers){};

bool RequestMessage::get_priority() const {
  return DynamicBoolHeader::read_value(priority);
}
void RequestMessage::set_priority(bool value) {
  if (DynamicBoolHeader::write_value(priority, DynamicHeader::PRIORITY,
                                     value)) {
    static_headers.message_size = 0;
  }
}

const std::string& RequestMessage::get_target_path() const {
  return DynamicStringHeader::read_value(target_path);
}
void RequestMessage::set_target_path(const std::string& value) {
  if (DynamicStringHeader::write_value(target_path, DynamicHeader::TARGET_PATH,
                                       value)) {
    static_headers.message_size = 0;
  }
}

const std::string& RequestMessage::get_permission_token() const {
  return DynamicStringHeader::read_value(permission_token);
}
void RequestMessage::set_permission_token(const std::string& value) {
  if (DynamicStringHeader::write_value(permission_token,
                                       DynamicHeader::PERMISSION_TOKEN, value)) {
    static_headers.message_size = 0;
  }
}

bool RequestMessage::get_no_stream() const {
  return DynamicBoolHeader::read_value(no_stream);
}
void RequestMessage::set_no_stream(bool value) {
  if (DynamicBoolHeader::write_value(no_stream, DynamicHeader::NO_STREAM,
                                     value)) {
    static_headers.message_size = 0;
  }
}

uint8_t RequestMessage::get_alias_count() const {
  return DynamicByteHeader::read_value(alias_count);
}
void RequestMessage::set_alias_count(uint8_t value) {
  if (DynamicByteHeader::write_value(alias_count, DynamicHeader::ALIAS_COUNT,
                                     value)) {
    static_headers.message_size = 0;
  }
}

ResponseMessage::ResponseMessage(const uint8_t* data, size_t size)
    : Message(data, size){};
ResponseMessage::ResponseMessage(MessageType type) : Message(type){};
ResponseMessage::ResponseMessage(const StaticHeaders& headers)
    : Message(headers){};

const std::string& ResponseMessage::get_source_path() const {
  return DynamicStringHeader::read_value(source_path);
}

void ResponseMessage::set_source_path(const std::string& value) {
  if (DynamicStringHeader::write_value(source_path, DynamicHeader::SOURCE_PATH,
                                       value)) {
    static_headers.message_size = 0;
  }
}

MessageStatus ResponseMessage::get_status() const {
  return MessageStatus(DynamicByteHeader::read_value(status));
}
void ResponseMessage::set_status(MessageStatus value) {
  if (DynamicByteHeader::write_value(status, DynamicHeader::STATUS, uint8_t(value))) {
    static_headers.message_size = 0;
  }
}

}  // namespace dsa
