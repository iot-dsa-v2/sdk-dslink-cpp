#ifndef DSA_SDK_F0_MESSAGE_H
#define DSA_SDK_F0_MESSAGE_H

#include <stdexcept>
#include "../base_message.h"

namespace dsa {

class HandshakeF0Message : public Message {
 public:
  uint8_t dsa_version_major;
  uint8_t dsa_version_minor;
  uint8_t dsid_length;
  std::string dsid;
  std::vector<uint8_t> public_key;
  bool security_preference;
  std::vector<uint8_t> salt;

  HandshakeF0Message(const uint8_t* data, size_t size);
  HandshakeF0Message();
  HandshakeF0Message(const HandshakeF0Message&);

 protected:
  // measure the size and header size
  void update_static_header();
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const;
  void parse_dynamic_headers(const uint8_t* data,
                             size_t size) throw(const MessageParsingError&);
};
}

#endif  // DSA_SDK_F0_MESSAGE_H
