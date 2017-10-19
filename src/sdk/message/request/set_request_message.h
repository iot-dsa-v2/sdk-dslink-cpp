#ifndef DSA_SDK_SET_REQUEST_MESSAGE_H_
#define DSA_SDK_SET_REQUEST_MESSAGE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <message/message_options.h>
#include "../../util/buffer.h"
#include "../base_message.h"
#include "../message_value.h"

namespace dsa {

class SetRequestMessage : public RequestMessage, PagedMessageMixin {
 public:
  SetRequestMessage(const uint8_t* data, size_t size);
  SetRequestMessage();
  SetRequestMessage(const SetRequestMessage&);

  std::unique_ptr<DynamicStringHeader> attribute_field;

 protected:
  // measure the size and header size
  void update_static_header();
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const;
  void parse_dynamic_data(const uint8_t* data, size_t dynamic_header_size,
                          size_t body_size) throw(const MessageParsingError&);

 public:
  MessageValue get_value() const;
  void set_value(MessageValue&& value);

  const std::string& get_attribute_field() const;
  void set_attribute_field(const std::string& value);
};

typedef ref_<const SetRequestMessage> SetRequestMessageCRef;

}  // namespace dsa

#endif  // DSA_SDK_SET_REQUEST_MESSAGE_H_
