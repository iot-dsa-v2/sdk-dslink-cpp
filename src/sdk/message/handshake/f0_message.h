#ifndef DSA_SDK_F0_MESSAGE_H
#define DSA_SDK_F0_MESSAGE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <stdexcept>
#include "../base_message.h"

namespace dsa {

class HandshakeF0Message final : public Message {
 public:
  uint8_t dsa_version_major = DSA_MAJOR_VERSION;
  uint8_t dsa_version_minor = DSA_MINOR_VERSION;
  //bool security_preference = false;

  string_ dsid;
  std::vector<uint8_t> public_key;
  std::vector<uint8_t> salt;

  HandshakeF0Message(const uint8_t* data, size_t size);
  HandshakeF0Message();

 protected:
  // measure the size and header size
  void update_static_header() final;
  //TODO void print_headers(std::ostream &os) const final;
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const final;
  void parse_dynamic_headers(const uint8_t* data,
                             size_t size) throw(const MessageParsingError&);
};
}

#endif  // DSA_SDK_F0_MESSAGE_H
