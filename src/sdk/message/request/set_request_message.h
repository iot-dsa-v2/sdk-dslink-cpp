#ifndef DSA_SDK_SET_REQUEST_MESSAGE_H
#define DSA_SDK_SET_REQUEST_MESSAGE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <message/message_options.h>
#include "../../util/buffer.h"
#include "../base_message.h"
#include "../message_value.h"

namespace dsa {

class SetRequestMessage final : public RequestMessage {
 public:
  SetRequestMessage(const uint8_t* data, size_t size);
  SetRequestMessage();
  SetRequestMessage(const SetRequestMessage&);
  SetRequestMessage(const string_& path, Var && value);

  std::unique_ptr<DynamicStringHeader> attribute_field;

 protected:
  mutable std::unique_ptr<MessageValue> _cached_value;
  // measure the size and header size
  void update_static_header() final;
  void print_headers(std::ostream &os) const final;
  void print_body(std::ostream& os) const final;

  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const final;
  void parse_dynamic_data(const uint8_t* data, size_t dynamic_header_size,
                          size_t body_size) throw(const MessageParsingError&);

 public:
  MessageValue get_value() const;
  void set_value(MessageValue&& value, int32_t sequence_id = 0);

  const string_& get_attribute_field() const;
  void set_attribute_field(const string_& value);
};

typedef ref_<const SetRequestMessage> SetRequestMessageCRef;

}  // namespace dsa

#endif  // DSA_SDK_SET_REQUEST_MESSAGE_H
