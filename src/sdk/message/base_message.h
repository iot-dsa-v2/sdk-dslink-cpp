#ifndef DSA_SDK_BASE_MESSAGE_H_
#define DSA_SDK_BASE_MESSAGE_H_

#include <memory>

#include "../util/buffer.h"
#include "dynamic_header.h"
#include "static_header.h"

namespace dsa {

class Message {
 public:
  StaticHeaders static_headers;

  std::unique_ptr<MessageBuffer> body;
  std::unique_ptr<DynamicByteHeader> priority;
};

class RequestMessage : public Message {
 public:
  std::unique_ptr<DynamicStringHeader> target_path;
  std::unique_ptr<DynamicStringHeader> permission_token;
  std::unique_ptr<DynamicBoolHeader> no_stream;
};

class ResponseMessage : public Message {
 public:
  std::unique_ptr<DynamicByteHeader> status;
};

}  // namespace dsa

#endif  // DSA_SDK_BASE_MESSAGE_H_
