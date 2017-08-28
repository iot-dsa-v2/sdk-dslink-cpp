//
// Created by rinick on 8/24/17.
//

#include "f2_message.h"

#include "util/little_endian.h"

namespace dsa {

HandshakeF2Message::HandshakeF2Message(const uint8_t* data, size_t size)
    : Message(data, size) {
  parse_dynamic_headers(data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

HandshakeF2Message::HandshakeF2Message()
    : Message(MessageType::Handshake2),
      is_requester(false),
      is_responder(false),
      auth(AuthLength) {}

void HandshakeF2Message::update_static_header() {
  static_headers.header_size = (uint16_t)StaticHeaders::TotalSize;
  static_headers.message_size =
      StaticHeaders::TotalSize +
      6 /* token_length, is_requester, is_responder, session_id_length */ +
      token.length() + session_id.length() + AuthLength;
}

void HandshakeF2Message::write_dynamic_data(uint8_t* data) const {
  data += write_str_with_len(data, token);
  (*data++) = (uint8_t)(is_requester ? 1 : 0);
  (*data++) = (uint8_t)(is_responder ? 1 : 0);
  data += write_str_with_len(data, session_id);
  data = std::copy(auth.begin(), auth.end(), data);
}

void HandshakeF2Message::parse_dynamic_headers(
    const uint8_t* data, size_t size) throw(const MessageParsingError&) {
  data += read_str_with_len(data, token);

  is_requester = (*data++ != 0u);
  is_responder = (*data++ != 0u);

  data += read_str_with_len(data, session_id);

  auth.assign(data, data + AuthLength);
}

}  // namespace dsa
