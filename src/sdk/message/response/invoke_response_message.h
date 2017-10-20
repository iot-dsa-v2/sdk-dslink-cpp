#ifndef DSA_SDK_INVOKE_RESPONSE_MESSAGE_H_
#define DSA_SDK_INVOKE_RESPONSE_MESSAGE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {
class Var;

class InvokeResponseMessage : public ResponseMessage, PagedMessageMixin {
 public:
  std::unique_ptr<DynamicBoolHeader> skippable;

  InvokeResponseMessage(const uint8_t* data, size_t size);
  InvokeResponseMessage();
  InvokeResponseMessage(const InvokeResponseMessage&);

 protected:
  // measure the size and header size
  void update_static_header();
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const;
  void parse_dynamic_data(const uint8_t* data, size_t dynamic_header_size,
                          size_t body_size) throw(const MessageParsingError&);

 public:
  void set_value(const Var& value);
};

typedef ref_<const InvokeResponseMessage> InvokeResponseMessageCRef;

}  // namespace dsa

#endif  // DSA_SDK_INVOKE_RESPONSE_MESSAGE_H_
