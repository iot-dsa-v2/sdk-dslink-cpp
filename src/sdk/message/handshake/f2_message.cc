//
// Created by rinick on 8/24/17.
//

#include "f2_message.h"

namespace dsa {

HandshakeF2Message::HandshakeF2Message(const uint8_t* data, size_t size)
    : Message(data, size) {
  parse_dynamic_headers(data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

HandshakeF2Message::HandshakeF2Message() : Message(MessageType::Handshake2) {
  token_length = 0;
  token.resize(0);
  is_requester = false;
  is_responder = false;
  session_id_length = 0;
  session_id.resize(0);
  auth.resize(AuthLength, ' ');
}


void HandshakeF2Message::update_static_header() {
  uint32_t header_size = StaticHeaders::TotalSize + sizeof(token_length) +
                         token_length + 2 + sizeof(session_id_length) +
                         session_id_length + AuthLength;

  static_headers.message_size = header_size;
  static_headers.header_size = (uint16_t)header_size;
}

void HandshakeF2Message::write_dynamic_data(uint8_t* data) const {
  data = std::copy(&token_length, &token_length + sizeof(token_length), data);
  if (token_length > 0) {
    data += token.copy(reinterpret_cast<char*>(data), token_length);
  }
  (*data++) = (uint8_t)(is_requester ? 1 : 0);
  (*data++) = (uint8_t)(is_responder ? 1 : 0);
  data = std::copy(&session_id_length,
                   &session_id_length + sizeof(session_id_length), data);
  if (session_id_length > 0) {
    data += session_id.copy(reinterpret_cast<char*>(data), session_id_length);
  }
  data = std::copy(auth.begin(), auth.end(), data);
}

void HandshakeF2Message::parse_dynamic_headers(
    const uint8_t* data, size_t size) throw(const MessageParsingError&) {
  std::copy(data, data + sizeof(token_length), &token_length);
  data += sizeof(token_length);

  // prevent accidental read in unowned memory
  //  if ((data - _write_buffer.data()) + token_length + 2 +
  //          sizeof(session_id_length) >
  //      size)
  //    return false;

  data +=
      token.assign(reinterpret_cast<const char*>(data), token_length).size();
  is_requester = (*data++ != 0u);
  is_responder = (*data++ != 0u);

  std::copy(data, data + sizeof(session_id_length), &session_id_length);
  data += sizeof(session_id_length);

  // prevent accidental read in unowned memory
  // if ((data - _write_buffer.data()) + session_id_length + AuthLength != size)
  //    return false;

  data +=
      session_id.assign(reinterpret_cast<const char*>(data), session_id_length)
          .size();
  auth.assign(data, data + AuthLength);
}

}  // namespace dsa
