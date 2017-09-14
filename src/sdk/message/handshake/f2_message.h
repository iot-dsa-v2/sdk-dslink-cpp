#ifndef DSA_SDK_F2_MESSAGE_H
#define DSA_SDK_F2_MESSAGE_H

#include <stdexcept>
#include "../base_message.h"

namespace dsa {

class HandshakeF2Message : public Message {
 public:
  bool is_requester; // TODO remove
  bool is_responder;

  std::string token;
  std::string previous_session_id;
  int32_t last_ack_id;
  std::vector<uint8_t> auth;

  HandshakeF2Message(const uint8_t* data, size_t size);
  HandshakeF2Message();


 protected:
  // measure the size and header size
  void update_static_header();
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const;
  void parse_dynamic_headers(const uint8_t* data,
                             size_t size) throw(const MessageParsingError&);
};
}

#endif  // DSA_SDK_F2_MESSAGE_H
