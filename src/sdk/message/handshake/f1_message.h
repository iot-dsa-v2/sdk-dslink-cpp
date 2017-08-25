#ifndef DSA_SDK_F1_MESSAGE_H
#define DSA_SDK_F1_MESSAGE_H

#include "../base_message.h"
#include <stdexcept>

namespace dsa {

class HandshakeF1Message : public Message {
 public:
  uint8_t dsid_length;
  std::string dsid;
  std::vector<uint8_t> public_key;
  std::vector<uint8_t> salt;

  HandshakeF1Message(const uint8_t* data, size_t size);
  HandshakeF1Message();
  //  HandshakeF1Message(const HandshakeF1Message&);

 protected:
  // measure the size and header size
  void update_static_header();
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const;
  void parse_dynamic_headers(const uint8_t* data,
                             size_t size) throw(const MessageParsingError&);
};
}

#endif  // DSA_SDK_F1_MESSAGE_H
