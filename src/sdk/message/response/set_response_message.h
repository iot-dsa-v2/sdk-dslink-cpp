#ifndef DSA_SDK_SET_RESPONSE_MESSAGE_H
#define DSA_SDK_SET_RESPONSE_MESSAGE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {

class SetResponseMessage final : public ResponseMessage {
 public:
  SetResponseMessage(const uint8_t* data, size_t size);
  SetResponseMessage();
  SetResponseMessage(const SetResponseMessage&);

 protected:
  // measure the size and header size
  void update_static_header() final;
  void print_headers(std::ostream &os) const final;
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const final;
  void parse_dynamic_data(const uint8_t* data, size_t dynamic_header_size,
                          size_t body_size) throw(const MessageParsingError&);
};

typedef ref_<const SetResponseMessage> SetResponseMessageCRef;

}  // namespace dsa

#endif  // DSA_SDK_SET_RESPONSE_MESSAGE_H
