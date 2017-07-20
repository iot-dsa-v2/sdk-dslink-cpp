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

  StaticHeaders static_headers;

  std::unique_ptr<SharedBuffer> body;

  std::unique_ptr<DynamicByteHeader> priority;

  explicit Message(const SharedBuffer &buffer);
 protected:
  virtual void parseDynamicHeaders(const uint8_t *data, size_t size) = 0;
};

class RequestMessage : public Message {
 public:
  explicit RequestMessage(const SharedBuffer &buffer);
  std::unique_ptr<DynamicStringHeader> target_path;
  std::unique_ptr<DynamicStringHeader> permission_token;
  std::unique_ptr<DynamicBoolHeader> no_stream;
};

class ResponseMessage : public Message {
 public:
  explicit ResponseMessage(const SharedBuffer &buffer);
  std::unique_ptr<DynamicByteHeader> status;
};

}  // namespace dsa

#endif  // DSA_SDK_BASE_MESSAGE_H_
