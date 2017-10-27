#ifndef DSA_SDK_F1_MESSAGE_H
#define DSA_SDK_F1_MESSAGE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <stdexcept>
#include "../base_message.h"

namespace dsa {

class HandshakeF1Message final : public Message {
 public:
  string_ dsid;
  std::vector<uint8_t> public_key;
  std::vector<uint8_t> salt;

  HandshakeF1Message(const uint8_t* data, size_t size);
  HandshakeF1Message();

 protected:
  // measure the size and header size
  void update_static_header() final;
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const final;
  void parse_dynamic_headers(const uint8_t* data,
                             size_t size) throw(const MessageParsingError&);
};
}

#endif  // DSA_SDK_F1_MESSAGE_H
