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
#include "util/enable_ref.h"
#include "util/path.h"

namespace dsa {

class Message : public EnableRef<Message> {
 public:
  enum : size_t {
    MAX_MESSAGE_SIZE =
        65472  // 65536-64, reserve 64 bytes for low level protocol headers.
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

  void set_body(RefCountBytes* b) {
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
  void set_target_path(const string_& value);

  const string_& get_permission_token() const;
  void set_permission_token(const string_& value);

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

  const string_& get_source_path() const;
  void set_source_path(const string_& value);

  MessageStatus get_status() const;
  void set_status(MessageStatus value);
};

typedef std::function<void(bool)> AckCallback;

class SubscribeResponseMessage;
struct SubscribeOptions;
class MessageStream : public DestroyableRef<MessageStream> {
 public:
  const int32_t rid;
  PermissionLevel allowed_permission;

  explicit MessageStream(int32_t rid) : rid(rid){};
  virtual ~MessageStream() = default;

  // write message to remote
  virtual size_t peek_next_message_size(size_t available, int64_t time) = 0;
  virtual MessageCRef get_next_message(AckCallback&) = 0;

  // read message from remote
  virtual void receive_message(ref_<Message>&& msg) = 0;

  // because fake stream also need to extend ref_ , define the following
  // interface here make things simpler. in other programing language these
  // functions should be defined in real interface

  // interface for fake outgoing list stream

  typedef std::function<void(MessageStream&)> ListCloseCallback;
  virtual void update_list_value(const string_& key, const BytesRef& value) {}
  virtual void on_list_close(ListCloseCallback&& callback){};

  // interface for fake outgoing subscribe stream

  typedef std::function<void(MessageStream&, const SubscribeOptions&)>
      SubOptionChangeCallback;
  virtual void send_subscribe_response(
      ref_<const SubscribeResponseMessage>&& message) {}
  virtual const SubscribeOptions& subscribe_options();
  virtual void on_subscribe_option_change(SubOptionChangeCallback&& callback){};
};
typedef MessageStream BaseOutgoingSubscribeStream;
typedef MessageStream BaseOutgoingListStream;

}  // namespace dsa

#endif  // DSA_SDK_BASE_MESSAGE_H_
