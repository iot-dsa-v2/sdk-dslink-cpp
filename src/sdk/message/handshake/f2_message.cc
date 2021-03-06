#include "dsa_common.h"

#include "f2_message.h"

#include "util/little_endian.h"

namespace dsa {

HandshakeF2Message::HandshakeF2Message(const uint8_t* data, size_t size)
    : Message(data, size) {
  parse_dynamic_headers(
      data + StaticHeaders::SHORT_TOTAL_SIZE,
      static_headers.header_size - StaticHeaders::SHORT_TOTAL_SIZE);
}

HandshakeF2Message::HandshakeF2Message()
    : Message(MessageType::HANDSHAKE2),
      is_responder(false),
      auth(AUTH_LENGTH) {}

void HandshakeF2Message::update_static_header() {
  static_headers.header_size = (uint16_t)StaticHeaders::SHORT_TOTAL_SIZE;
  static_headers.message_size =
      StaticHeaders::SHORT_TOTAL_SIZE +
      5 /* token_length, is_responder*/ + path.length() +
      token.length() + AUTH_LENGTH;
}

void HandshakeF2Message::write_dynamic_data(uint8_t* data) const {
  data += write_str_with_len(data, token);
  (*data++) = (uint8_t)(is_responder ? 1 : 0);
  data += write_str_with_len(data, path);
  data = std::copy(auth.begin(), auth.end(), data);
}

void HandshakeF2Message::parse_dynamic_headers(
    const uint8_t* data, size_t size) throw(const MessageParsingError&) {
  data += read_str_with_len(data, token);
  is_responder = (*data++ != 0u);
  data += read_str_with_len(data, path);
  auth.assign(data, data + AUTH_LENGTH);
}

}  // namespace dsa
