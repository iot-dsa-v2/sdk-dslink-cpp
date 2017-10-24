#ifndef DSA_SDK_BASE_MESSAGE_H_
#define DSA_SDK_BASE_MESSAGE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <vector>

#include "dynamic_header.h"
#include "enums.h"
#include "static_headers.h"
#include "util/buffer.h"
#include "util/enable_intrusive.h"
#include "util/path.h"

namespace dsa {

class Message : public EnableRef<Message> {
 public:
  enum : size_t {
    MAX_MESSAGE_SIZE = 65472 // 65536-64, reserve 64 bytes for low level protocol headers.
  };

  static MessageType get_response_type(MessageType request_type) {
    if (static_cast<uint8_t>(request_type) > 0 &&
        static_cast<uint8_t>(request_type) < 0x10) {
      return static_cast<MessageType>(static_cast<uint8_t>(request_type) |
                                      0x80);
    }
    return MessageType::INVALID;
  }

  static const uint8_t PUBLIC_KEY_LENGTH = 65;
  static const uint8_t SALT_LENGTH = 32;
  static const uint8_t AUTH_LENGTH = 32;

 public:
  static ref_<Message> parse_message(const uint8_t* data, size_t size) throw(
      const MessageParsingError&);

 public:
  explicit Message(const uint8_t* data, size_t size);
  Message(MessageType type);
  Message(const StaticHeaders& headers);
  virtual ~Message() = default;

  const int64_t created_ts;
  int32_t size() const;

  // update_static_header must be called before write
  // void write(uint8_t* data) const throw(const MessageParsingError&);
  void write(uint8_t* data, int32_t rid = 0, int32_t ack_id = 0) const
      throw(const MessageParsingError&);

  int32_t get_sequence_id() const;
  void set_sequence_id(int32_t value);

  int32_t get_page_id() const;
  void set_page_id(int32_t value);

  MessageType type() const { return static_headers.type; }
  bool is_request() const {
    return static_cast<uint8_t>(static_headers.type) < 0x80;
  }
  bool need_ack() { return static_cast<uint8_t>(static_headers.type) < 0xF0; }

  int32_t get_rid() { return static_headers.rid; }
  void set_rid(int32_t rid) { static_headers.rid = rid; }

  int32_t get_ack_id() const { return static_headers.ack_id; }
  void set_ack_rid(int32_t ack_id) { static_headers.ack_id = ack_id; }

  void set_body(IntrusiveBytes* b) {
    static_headers.message_size = 0;  // invalidate message_size
    body.reset(b);
  }
  BytesRef get_body() const { return body; }

 protected:
  // measure the size and header size
  virtual void update_static_header();
  // write dynamic header and body
  virtual void write_dynamic_data(uint8_t* data) const {};

  StaticHeaders static_headers;

  BytesRef body;

  std::unique_ptr<DynamicIntHeader> sequence_id;
  std::unique_ptr<DynamicIntHeader> page_id;
};
typedef ref_<Message> MessageRef;
typedef ref_<const Message> MessageCRef;

class PagedMessageMixin {
 public:
  PagedMessageMixin();

 protected:
  std::vector<ref_<Message>> other_pages;
  size_t current_page;
};

class RequestMessage : public Message {
 protected:
  std::unique_ptr<DynamicBoolHeader> priority;
  std::unique_ptr<DynamicStringHeader> target_path;
  std::unique_ptr<DynamicStringHeader> permission_token;
  std::unique_ptr<DynamicBoolHeader> no_stream;
  std::unique_ptr<DynamicByteHeader> alias_count;

  mutable std::unique_ptr<const Path> _parsed_target_path;

 public:
  explicit RequestMessage(const uint8_t* data, size_t size);
  explicit RequestMessage(MessageType type);
  explicit RequestMessage(const StaticHeaders& headers);

  bool get_priority() const;
  void set_priority(bool value);

  const Path& get_target_path() const;
  void set_target_path(const std::string& value);

  const std::string& get_permission_token() const;
  void set_permission_token(const std::string& value);

  bool get_no_stream() const;
  void set_no_stream(bool value);

  uint8_t get_alias_count() const;
  void set_alias_count(uint8_t value);
};

class ResponseMessage : public Message {
 protected:
  std::unique_ptr<DynamicStringHeader> source_path;
  std::unique_ptr<DynamicByteHeader> status;

 public:
  explicit ResponseMessage(const uint8_t* data, size_t size);
  explicit ResponseMessage(MessageType type);
  explicit ResponseMessage(const StaticHeaders& headers);

  const std::string& get_source_path() const;
  void set_source_path(const std::string& value);

  MessageStatus get_status() const;
  void set_status(MessageStatus value);
};

typedef std::function<void(bool)> AckCallback;

class MessageStream : public DestroyableRef<MessageStream> {
 public:
  const int32_t rid;

  explicit MessageStream(int32_t rid) : rid(rid){};
  virtual ~MessageStream() = default;

  // write message to remote
  virtual size_t peek_next_message_size(size_t available, int64_t time) = 0;
  virtual MessageCRef get_next_message(AckCallback&) = 0;

  // read message from remote
  virtual void receive_message(MessageCRef&& msg) = 0;
};

}  // namespace dsa

#endif  // DSA_SDK_BASE_MESSAGE_H_
