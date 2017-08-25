//
// Created by rinick on 8/24/17.
//

#include "f0_message.h"

namespace dsa {

HandshakeF0Message::HandshakeF0Message(const uint8_t* data, size_t size)
    : Message(data, size) {
  parse_dynamic_headers(data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

HandshakeF0Message::HandshakeF0Message() : Message(MessageType::Handshake0) {}

HandshakeF0Message::HandshakeF0Message(const HandshakeF0Message& from)
    : Message{from.static_headers} {}

void HandshakeF0Message::update_static_header() {
  uint32_t header_size =
      StaticHeaders::TotalSize + PublicKeyLength + SaltLength + 5;

  static_headers.message_size = header_size;
  static_headers.header_size = (uint16_t)header_size;
}

void HandshakeF0Message::write_dynamic_data(uint8_t* data) const {
  (*data++) = dsa_version_major;
  (*data++) = dsa_version_minor;
  data = std::copy(&dsid_length, &dsid_length + sizeof(dsid_length), data);
  data += dsid.copy(reinterpret_cast<char*>(data), dsid_length);
  data = std::copy(public_key.begin(), public_key.end(), data);
  (*data++) = security_preference;
  data = std::copy(salt.begin(), salt.end(), data);
}

void HandshakeF0Message::parse_dynamic_headers(
    const uint8_t* data, size_t size) throw(const MessageParsingError&) {
  // TODO : validate the message
  //  if (!valid_handshake_header(header, size, MessageType::Handshake0))
  //    return false;

  dsa_version_major = *data++;
  dsa_version_minor = *data++;
  dsid_length = *data++;

  //  if ((data - _write_buffer.data()) + dsid_length + PublicKeyLength + 1 +
  //          SaltLength >
  //      size)
  //    return false;

  data += dsid.assign(reinterpret_cast<const char*>(data), dsid_length).size();
  public_key.assign(data, data + PublicKeyLength);
  data += public_key.size();

  security_preference = static_cast<bool>(*data++);

  salt.assign(data, data + SaltLength);
}
}  // namespace dsa
