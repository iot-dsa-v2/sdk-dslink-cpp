#ifndef DSA_SDK_BASE_MESSAGE_H_
#define DSA_SDK_BASE_MESSAGE_H_

#include <vector>

#include "dynamic_header.h"
#include "enums.h"
#include "static_headers.h"
#include "util/buffer.h"
#include "util/enable_intrusive.h"

namespace dsa {

class Message : public EnableIntrusive<Message> {
 public:
  explicit Message(const Buffer::SharedBuffer& buffer);
  Message(MessageType type);
  Message(const StaticHeaders& headers);

  uint32_t size() const;

  // update_static_header must be called before write
  void write(uint8_t* data) const throw(const MessageParsingError&);

  int32_t get_sequence_id() const;
  void set_sequence_id(int32_t value);

  int32_t get_page_id() const;
  void set_page_id(int32_t value);

  MessageType type() { return static_headers.type; }

  uint32_t request_id() { return static_headers.request_id; }

 protected:
  // measure the size and header size
  virtual void update_static_header() = 0;
  // write dynamic header and body
  virtual void write_dynamic_data(uint8_t* data) const = 0;

  StaticHeaders static_headers;

  std::unique_ptr<SharedBuffer> body;

  std::unique_ptr<DynamicIntHeader> sequence_id;
  std::unique_ptr<DynamicIntHeader> page_id;
};

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
  explicit RequestMessage(const SharedBuffer& buffer);
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
  explicit ResponseMessage(const SharedBuffer& buffer);
  explicit ResponseMessage(MessageType type);
  explicit ResponseMessage(const StaticHeaders& headers);

  const std::string& get_source_path() const;
  void set_source_path(const std::string& value);

  MessageStatus get_status() const;
  void set_status(MessageStatus value);
};

}  // namespace dsa

#endif  // DSA_SDK_BASE_MESSAGE_H_
