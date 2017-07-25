#ifndef DSA_SDK_BASE_MESSAGE_H_
#define DSA_SDK_BASE_MESSAGE_H_

#include <memory>

#include "../util/buffer.h"
#include "dynamic_header.h"
#include "static_header.h"

namespace dsa {

class Message {
 public:
  enum {  // https://github.com/iot-dsa-v2/docs/blob/master/protocol/Header-Structure.md
    SUBSCRIBE_REQUEST_TYPE = 0x01,
    LIST_REQUEST_TYPE = 0x02,
    INVOKE_REQUEST_TYPE = 0x03,
    SET_REQUEST_TYPE = 0x04,
    SUBSCRIBE_RESPONSE_TYPE = 0x81,
    LIST_RESPONSE_TYPE = 0x82,
    INVOKE_RESPONSE_TYPE = 0x83,
    SET_RESPONSE_TYPE = 0x84
  };

 public:
  explicit Message(const SharedBuffer& buffer);

  uint32_t size() const { return static_headers.message_size; }

  // update_static_header must be called before write
  void write(uint8_t* data) const throw(const std::runtime_error&);
  // measure the size and header size
  virtual void update_static_header() = 0;

 protected:
  // write dynamic header and body
  virtual void write_dynamic_data(uint8_t* data) const = 0;

 protected:
  StaticHeaders static_headers;

  std::unique_ptr<SharedBuffer> body;

  std::unique_ptr<DynamicByteHeader> priority;
  std::unique_ptr<DynamicIntHeader> sequence_id;
  std::unique_ptr<DynamicIntHeader> page_id;

 public:
  uint8_t get_priority() const;
  void set_priority(uint8_t value);

  int32_t get_sequence_id() const;
  void set_sequence_id(int32_t value);

  int32_t get_page_id() const;
  void set_page_id(int32_t value);
};

class RequestMessage : public Message {
 public:
  explicit RequestMessage(const SharedBuffer& buffer);

 protected:
  std::unique_ptr<DynamicStringHeader> target_path;
  std::unique_ptr<DynamicStringHeader> permission_token;
  std::unique_ptr<DynamicBoolHeader> no_stream;
  std::unique_ptr<DynamicByteHeader> alias_count;

 public:
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
 public:
  explicit ResponseMessage(const SharedBuffer& buffer);

 protected:
  std::unique_ptr<DynamicStringHeader> source_path;
  std::unique_ptr<DynamicByteHeader> status;

 public:
  bool get_source_path() const;
  void set_source_path(bool value);

  uint8_t get_status() const;
  void set_status(uint8_t value);
};

}  // namespace dsa

#endif  // DSA_SDK_BASE_MESSAGE_H_
