//
// Created by rinick on 8/24/17.
//

#include "f3_message.h"

namespace dsa {

HandshakeF3Message::HandshakeF3Message(const uint8_t* data, size_t size)
    : Message(data, size) {
  parse_dynamic_headers(data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

HandshakeF3Message::HandshakeF3Message() : Message(MessageType::Handshake3) {
  session_id_length = 0;
  path_length = 0;
  other_auth.resize(AuthLength, ' ');
}


void HandshakeF3Message::update_static_header() {
  uint32_t header_size = StaticHeaders::TotalSize + sizeof(session_id_length) +
                         session_id_length + sizeof(path_length) + path_length +
                         AuthLength;

  static_headers.message_size = header_size;
  static_headers.header_size = (uint16_t)header_size;
}

void HandshakeF3Message::write_dynamic_data(uint8_t* data) const {
  data = std::copy(&session_id_length,
                   &session_id_length + sizeof(session_id_length), data);
  if (session_id_length > 0) {
    data += session_id.copy(reinterpret_cast<char*>(data), session_id_length);
  }
  data = std::copy(&path_length, &path_length + sizeof(path_length), data);
  if (path_length > 0) {
    data += path.copy(reinterpret_cast<char*>(data), path_length);
  }
  data = std::copy(other_auth.begin(), other_auth.end(), data);
}

void HandshakeF3Message::parse_dynamic_headers(
    const uint8_t* data, size_t size) throw(const MessageParsingError&) {
  //  if (!valid_handshake_header(header, size, MessageType::Handshake3))
  //    return false;

  std::copy(data, data + sizeof(session_id_length), &session_id_length);
  data += sizeof(session_id_length);
  //  if (cur + session_id_length + sizeof(path_length) > size) return false;
  data +=
      session_id.assign(reinterpret_cast<const char*>(data), session_id_length)
          .size();

  std::copy(data, data + sizeof(path_length), &path_length);
  data += sizeof(path_length);
  // if (cur + path_length + AuthLength > size) return false;
  data += path.assign(reinterpret_cast<const char*>(data), path_length).size();
}

}  // namespace dsa
