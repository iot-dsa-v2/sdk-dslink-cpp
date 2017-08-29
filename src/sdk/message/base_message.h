#ifndef DSA_SDK_BASE_MESSAGE_H_
#define DSA_SDK_BASE_MESSAGE_H_

#include <vector>

#include "dynamic_header.h"
#include "enums.h"
#include "static_headers.h"
#include "util/buffer.h"
#include "util/enable_intrusive.h"
#include "variant/variant.h"

namespace dsa {

class Message : public EnableIntrusive<Message> {
 public:
  enum : size_t { MAX_MESSAGE_SIZE = 65536 };

  static const uint8_t PublicKeyLength = 65;
  static const uint8_t SaltLength = 32;
  static const uint8_t AuthLength = 32;

 public:
  static intrusive_ptr_<Message> parse_message(
      const uint8_t* data, size_t size) throw(const MessageParsingError&);

 public:
  explicit Message(const uint8_t* data, size_t size);
  Message(MessageType type);
  Message(const StaticHeaders& headers);
  virtual ~Message() = default;

  uint32_t size() const;

  // update_static_header must be called before write
  void write(uint8_t* data) const throw(const MessageParsingError&);

  int32_t get_sequence_id() const;
  void set_sequence_id(int32_t value);

  int32_t get_page_id() const;
  void set_page_id(int32_t value);

  MessageType type() const { return static_headers.type; }
  bool is_request() const {
    return static_cast<uint8_t>(static_headers.type) < 0x80;
  }

  uint32_t request_id() const { return static_headers.request_id; }

 protected:
  // measure the size and header size
  virtual void update_static_header() = 0;
  // write dynamic header and body
  virtual void write_dynamic_data(uint8_t* data) const = 0;

  StaticHeaders static_headers;

  BytesPtr body;

  std::unique_ptr<DynamicIntHeader> sequence_id;
  std::unique_ptr<DynamicIntHeader> page_id;
};
typedef intrusive_ptr_<Message> MessagePtr;

class PagedMessageMixin {
 public:
  PagedMessageMixin();

 protected:
  std::vector<intrusive_ptr_<Message>> other_pages;
  size_t current_page;
};

class RequestMessage : public Message {
 protected:
  std::unique_ptr<DynamicBoolHeader> priority;
  std::unique_ptr<DynamicStringHeader> target_path;
  std::unique_ptr<DynamicStringHeader> permission_token;
  std::unique_ptr<DynamicBoolHeader> no_stream;
  std::unique_ptr<DynamicByteHeader> alias_count;

 public:
  explicit RequestMessage(const uint8_t* data, size_t size);
  explicit RequestMessage(MessageType type);
  explicit RequestMessage(const StaticHeaders& headers);

  bool get_priority() const;
  void set_priority(bool value);

  const std::string& get_target_path() const;
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

class MessageStream : public IntrusiveClosable<MessageStream> {
 public:
  virtual ~MessageStream() = default;

  virtual size_t peek_next_message_size(size_t available) = 0;
  virtual MessagePtr get_next_message() = 0;
};

}  // namespace dsa

#endif  // DSA_SDK_BASE_MESSAGE_H_
