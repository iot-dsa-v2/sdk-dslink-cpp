#ifndef DSA_SDK_F2_MESSAGE_H
#define DSA_SDK_F2_MESSAGE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <stdexcept>
#include "../base_message.h"

namespace dsa {

class HandshakeF2Message final : public Message {
 public:
  bool is_responder;

  string_ token;
  int32_t last_ack_id;
  string_ path;
  std::vector<uint8_t> auth;

  HandshakeF2Message(const uint8_t* data, size_t size);
  HandshakeF2Message();

 protected:
  // measure the size and header size
  void update_static_header() final;
  // TODO void print_headers(std::ostream &os) const final;
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const final;
  void parse_dynamic_headers(const uint8_t* data,
                             size_t size) throw(const MessageParsingError&);
};
}

#endif  // DSA_SDK_F2_MESSAGE_H
