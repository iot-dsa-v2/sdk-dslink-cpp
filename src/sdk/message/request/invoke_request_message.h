#ifndef DSA_SDK_INVOKE_REQUEST_MESSAGE_H_
#define DSA_SDK_INVOKE_REQUEST_MESSAGE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <message/message_options.h>
#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {
class Var;

class InvokeRequestMessage final : public RequestMessage, PagedMessageMixin {
 public:
  InvokeRequestMessage(const uint8_t* data, size_t size);
  InvokeRequestMessage();
  InvokeRequestMessage(const InvokeRequestMessage&);

  std::unique_ptr<DynamicByteHeader> max_permission;

 protected:
  // measure the size and header size
  void update_static_header() final;
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const final;
  void parse_dynamic_data(const uint8_t* data, size_t dynamic_header_size,
                          size_t body_size) throw(const MessageParsingError&);

 public:
  void set_value(const Var& value);
};

typedef ref_<const InvokeRequestMessage> InvokeRequestMessageCRef;

}  // namespace dsa

#endif  // DSA_SDK_INVOKE_REQUEST_MESSAGE_H_
