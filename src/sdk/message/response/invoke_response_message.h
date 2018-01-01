#ifndef DSA_SDK_INVOKE_RESPONSE_MESSAGE_H_
#define DSA_SDK_INVOKE_RESPONSE_MESSAGE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {
class Var;

class InvokeResponseMessage final : public ResponseMessage {
 public:
  InvokeResponseMessage(const uint8_t* data, size_t size);
  InvokeResponseMessage();
  InvokeResponseMessage(const InvokeResponseMessage&);

 protected:
  std::unique_ptr<DynamicBoolHeader> skippable;
  std::unique_ptr<DynamicBoolHeader> refreshed;

  // measure the size and header size
  void update_static_header() final;
  void print_headers(std::ostream &os) const final;
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const final;
  void parse_dynamic_data(const uint8_t* data, size_t dynamic_header_size,
                          size_t body_size) throw(const MessageParsingError&);

 public:
  void set_value(const Var& value);
  Var get_value() const;

  const bool get_skippable() const;
  void set_skippable(bool value);

  const bool get_refreshed() const;
  void set_refreshed(bool value);
};

typedef ref_<const InvokeResponseMessage> InvokeResponseMessageCRef;

}  // namespace dsa

#endif  // DSA_SDK_INVOKE_RESPONSE_MESSAGE_H_
