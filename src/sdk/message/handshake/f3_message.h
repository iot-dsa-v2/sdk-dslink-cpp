#ifndef DSA_SDK_F3_MESSAGE_H
#define DSA_SDK_F3_MESSAGE_H

#include "../base_message.h"
#include <stdexcept>

namespace dsa {


class HandshakeF3Message : public Message {
 public:
  HandshakeF3Message(const uint8_t* data, size_t size);
  HandshakeF3Message();
//  HandshakeF3Message(const HandshakeF3Message&);

 protected:
  // measure the size and header size
  void update_static_header();
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const;
  void parse_dynamic_headers(const uint8_t* data,
                             size_t size) throw(const MessageParsingError&);
};

}

#endif  // DSA_SDK_F3_MESSAGE_H
