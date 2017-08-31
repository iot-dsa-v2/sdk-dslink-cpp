//
// Created by rinick on 8/24/17.
//

#include "f3_message.h"

#include "util/little_endian.h"

namespace dsa {

HandshakeF3Message::HandshakeF3Message(const uint8_t* data, size_t size)
    : Message(data, size) {
  parse_dynamic_headers(data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

HandshakeF3Message::HandshakeF3Message()
    : Message(MessageType::HANDSHAKE3), auth(AuthLength) {}

void HandshakeF3Message::update_static_header() {
  static_headers.header_size = (uint16_t)StaticHeaders::TotalSize;
  static_headers.message_size =
      StaticHeaders::TotalSize + 4 /* session_id_length + path_length */ +
      session_id.length() + path.length() + AuthLength;
}

void HandshakeF3Message::write_dynamic_data(uint8_t* data) const {
  data += write_str_with_len(data, session_id);
  data += write_str_with_len(data, path);
  data = std::copy(auth.begin(), auth.end(), data);
}

void HandshakeF3Message::parse_dynamic_headers(
    const uint8_t* data, size_t size) throw(const MessageParsingError&) {
  data += read_str_with_len(data, session_id);
  data += read_str_with_len(data, path);
  auth.assign(data, data + AuthLength);
}

}  // namespace dsa
