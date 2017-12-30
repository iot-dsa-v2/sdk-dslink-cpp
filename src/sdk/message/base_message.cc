#include "dsa_common.h"

#include "base_message.h"

#include <ctime>
#include <sstream>
#include "message_options.h"
#include "util/path.h"
#include "variant/variant.h"

namespace dsa {
Message::Message(const uint8_t* data, size_t size)
    : static_headers(data), created_ts(std::time(nullptr)){};
Message::Message(MessageType type)
    : static_headers(0, 0, type, 0, 0), created_ts(std::time(nullptr)){};
Message::Message(const StaticHeaders& headers)
    : static_headers(headers), created_ts(std::time(nullptr)){};

void Message::print_message(std::ostream& os, int32_t rid) const {
  os << type() << " (" << rid << ") SIZE:" << size();

  os << " [";
  print_headers(os);
  os << " ] ";
  print_body(os);
}
void Message::print_headers(std::ostream& os) const {}
void Message::print_body(std::ostream& os) const {
  if (body != nullptr && body->size() > 0) {
    if (body->size() < 128) {
      Var v = Var::from_msgpack(body->data(), body->size());
      if (!v.is_null()) {
        os << " BODY: " << v.to_json();
        return;
      }
    }
    os << " BODY SIZE: " << body->size();
  }
}

int32_t Message::size() const {
  if (!static_headers.message_size) {
    const_cast<Message*>(this)->update_static_header();
  }
  return static_headers.message_size;
}

void Message::update_static_header() {
  static_headers.message_size = StaticHeaders::TOTAL_SIZE;
  static_headers.header_size = StaticHeaders::TOTAL_SIZE;
}

void Message::write(uint8_t* data, int32_t rid, int32_t ack_id) const
    throw(const MessageParsingError&) {
  if (static_headers.message_size == 0) {
    // message_size shouldn't be 0
    throw MessageParsingError("invalid message size");
  }
  // write dynamic header and message body after the static header
  write_dynamic_data(static_headers.write(data, rid, ack_id));
}

void Message::set_sequence_id(int32_t value) {
  if (DynamicIntHeader::write_value(sequence_id, DynamicHeader::SEQUENCE_ID,
                                    value)) {
    static_headers.message_size = 0;
  }
}

void Message::set_page_id(int32_t value) {
  if (DynamicIntHeader::write_value(page_id, DynamicHeader::PAGE_ID, value)) {
    static_headers.message_size = 0;
  }
}

PagedMessageMixin::PagedMessageMixin() : current_page(SIZE_MAX) {}

RequestMessage::RequestMessage(const uint8_t* data, size_t size)
    : Message(data, size){};
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

const Path& RequestMessage::get_target_path() const {
  if (_parsed_target_path == nullptr) {
    _parsed_target_path.reset(
        new Path(DynamicStringHeader::read_value(target_path)));
  }
  return *_parsed_target_path;
}
void RequestMessage::set_target_path(const string_& value) {
  if (DynamicStringHeader::write_value(target_path, DynamicHeader::TARGET_PATH,
                                       value)) {
    _parsed_target_path.reset();
    static_headers.message_size = 0;
  }
}

const string_& RequestMessage::get_permission_token() const {
  return DynamicStringHeader::read_value(permission_token);
}
void RequestMessage::set_permission_token(const string_& value) {
  if (DynamicStringHeader::write_value(
          permission_token, DynamicHeader::PERMISSION_TOKEN, value)) {
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

const string_& ResponseMessage::get_source_path() const {
  return DynamicStringHeader::read_value(source_path);
}

void ResponseMessage::set_source_path(const string_& value) {
  if (DynamicStringHeader::write_value(source_path, DynamicHeader::SOURCE_PATH,
                                       value)) {
    static_headers.message_size = 0;
  }
}

void ResponseMessage::set_status(MessageStatus value) {
  if (DynamicByteHeader::write_value(status, DynamicHeader::STATUS,
                                     uint8_t(value))) {
    static_headers.message_size = 0;
  }
}

const SubscribeOptions& MessageStream::subscribe_options() {
  return SubscribeOptions::default_options;
}

}  // namespace dsa
