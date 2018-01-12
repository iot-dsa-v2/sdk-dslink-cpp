#include "dsa_common.h"

#include "f3_message.h"

#include "util/little_endian.h"

namespace dsa {

HandshakeF3Message::HandshakeF3Message(const uint8_t* data, size_t size)
    : Message(data, size) {
  parse_dynamic_headers(
      data + StaticHeaders::SHORT_TOTAL_SIZE,
      static_headers.header_size - StaticHeaders::SHORT_TOTAL_SIZE);
}

HandshakeF3Message::HandshakeF3Message()
    : Message(MessageType::HANDSHAKE3), auth(AUTH_LENGTH) {}

void HandshakeF3Message::update_static_header() {
  static_headers.header_size = (uint16_t)StaticHeaders::SHORT_TOTAL_SIZE;
  static_headers.message_size = StaticHeaders::SHORT_TOTAL_SIZE +
                                7 /* allow_requester  +
  + last_ack_id + path_length */ +
                                path.length() + AUTH_LENGTH;
}

void HandshakeF3Message::write_dynamic_data(uint8_t* data) const {
  (*data++) = (uint8_t)(allow_requester ? 1 : 0);
  data += write_32_t(data, last_ack_id);
  data += write_str_with_len(data, path);
  data = std::copy(auth.begin(), auth.end(), data);
}

void HandshakeF3Message::parse_dynamic_headers(
    const uint8_t* data, size_t size) throw(const MessageParsingError&) {
  allow_requester = (*data++ != 0u);
  last_ack_id = read_32_t(data);
  data += sizeof(uint32_t);
  data += read_str_with_len(data, path);
  auth.assign(data, data + AUTH_LENGTH);
}

}  // namespace dsa
