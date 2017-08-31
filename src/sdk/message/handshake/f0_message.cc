//
// Created by rinick on 8/24/17.
//

#include "f0_message.h"

#include "util/little_endian.h"

namespace dsa {

HandshakeF0Message::HandshakeF0Message(const uint8_t* data, size_t size)
    : Message(data, size), public_key(PublicKeyLength), salt(SaltLength) {
  parse_dynamic_headers(data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

HandshakeF0Message::HandshakeF0Message()
    : Message(MessageType::HANDSHAKE0),
      public_key(PublicKeyLength),
      salt(SaltLength) {}

void HandshakeF0Message::update_static_header() {
  static_headers.header_size = (uint16_t)StaticHeaders::TotalSize;
  static_headers.message_size = StaticHeaders::TotalSize + 2 /*version*/ +
                                2 /*dsidlen*/ + dsid.length() +
                                PublicKeyLength + SaltLength + 1 /*encryption*/;
}

void HandshakeF0Message::write_dynamic_data(uint8_t* data) const {
  (*data++) = dsa_version_major;
  (*data++) = dsa_version_minor;
  data += write_str_with_len(data, dsid);
  data = std::copy(public_key.begin(), public_key.end(), data);
  (*data++) = static_cast<uint8_t>(security_preference);
  data = std::copy(salt.begin(), salt.end(), data);
}

void HandshakeF0Message::parse_dynamic_headers(
    const uint8_t* data, size_t size) throw(const MessageParsingError&) {
  dsa_version_major = *data++;
  dsa_version_minor = *data++;

  data += read_str_with_len(data, dsid);

  public_key.assign(data, data + PublicKeyLength);
  data += PublicKeyLength;

  security_preference = static_cast<bool>(*data++);

  salt.assign(data, data + SaltLength);
}

}  // namespace dsa
