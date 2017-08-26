//
// Created by rinick on 8/24/17.
//

#include "f1_message.h"

namespace dsa {

HandshakeF1Message::HandshakeF1Message(const uint8_t* data, size_t size)
    : Message(data, size) {
  parse_dynamic_headers(data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

HandshakeF1Message::HandshakeF1Message() : Message(MessageType::Handshake1) {
  dsid_length = 0;
  dsid.resize(0);
  public_key.resize(PublicKeyLength, ' ');
  salt.resize(SaltLength, ' ');
}

HandshakeF1Message::HandshakeF1Message(const HandshakeF1Message& from)
    : Message{from.static_headers} {
  dsid_length = from.dsid_length;
  dsid = from.dsid;
  public_key = from.public_key;
  salt = from.salt;
}

void HandshakeF1Message::update_static_header() {
  uint32_t header_size =
      StaticHeaders::TotalSize + 1 + dsid_length + PublicKeyLength + SaltLength;

  static_headers.message_size = header_size;
  static_headers.header_size = (uint16_t)header_size;
}

void HandshakeF1Message::write_dynamic_data(uint8_t* data) const {
  data = std::copy(&dsid_length, &dsid_length + sizeof(dsid_length), data);
  if (dsid_length > 0) {
    data += dsid.copy(reinterpret_cast<char*>(data), dsid_length);
  }
  data = std::copy(public_key.begin(), public_key.end(), data);
  data = std::copy(salt.begin(), salt.end(), data);
}

void HandshakeF1Message::parse_dynamic_headers(
    const uint8_t* data, size_t size) throw(const MessageParsingError&) {
  // TODO : validate the message
  //  if (!valid_handshake_header(header, size, MessageType::Handshake0))
  //    return false;

  dsid_length = *data++;

  //  if ((data - _write_buffer.data()) + dsid_length + PublicKeyLength + 1 +
  //          SaltLength >
  //      size)
  //    return false;

  data += dsid.assign(reinterpret_cast<const char*>(data), dsid_length).size();
  public_key.assign(data, data + PublicKeyLength);
  data += public_key.size();

  salt.assign(data, data + SaltLength);
}

}  // namespace dsa
