#ifndef DSA_SDK_INVOKE_REQUEST_MESSAGE_H
#define DSA_SDK_INVOKE_REQUEST_MESSAGE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <message/message_options.h>
#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {
class Var;

class InvokeRequestMessage final : public RequestMessage {
 public:
  InvokeRequestMessage(const uint8_t* data, size_t size);
  InvokeRequestMessage();
  InvokeRequestMessage(const InvokeRequestMessage&);
  InvokeRequestMessage(const string_& path, Var && value);

  std::unique_ptr<DynamicByteHeader> max_permission;

 protected:
  std::unique_ptr<DynamicBoolHeader> skippable;
  std::unique_ptr<DynamicBoolHeader> refreshed;

  // measure the size and header size
  void update_static_header() final;
  void print_headers(std::ostream& os) const final;
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const final;
  void parse_dynamic_data(const uint8_t* data, size_t dynamic_header_size,
                          size_t body_size) throw(const MessageParsingError&);

 public:
  // return true when it's a multi-page message
  bool set_value(const Var& value, int32_t sequence_id = 0);
  Var get_value() const;

  const PermissionLevel get_max_permission() const;
  void set_max_permission(PermissionLevel value);

  const bool get_skippable() const;
  void set_skippable(bool value);

  const bool get_refreshed() const;
  void set_refreshed(bool value);
};

typedef ref_<const InvokeRequestMessage> InvokeRequestMessageCRef;

}  // namespace dsa

#endif  // DSA_SDK_INVOKE_REQUEST_MESSAGE_H
