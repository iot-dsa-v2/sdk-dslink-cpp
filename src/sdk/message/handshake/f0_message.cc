#include "dsa_common.h"

#include "f0_message.h"

#include "util/little_endian.h"

namespace dsa {

HandshakeF0Message::HandshakeF0Message(const uint8_t* data, size_t size)
    : Message(data, size), public_key(PUBLIC_KEY_LENGTH), salt(SALT_LENGTH) {
  parse_dynamic_headers(data + StaticHeaders::TOTAL_SIZE,
                        static_headers.header_size - StaticHeaders::TOTAL_SIZE);
}

HandshakeF0Message::HandshakeF0Message()
    : Message(MessageType::HANDSHAKE0),
      public_key(PUBLIC_KEY_LENGTH),
      salt(SALT_LENGTH) {}

void HandshakeF0Message::update_static_header() {
  static_headers.header_size = (uint16_t)StaticHeaders::TOTAL_SIZE;
  static_headers.message_size = StaticHeaders::TOTAL_SIZE + 2 /*version*/ +
                                2 /*dsidlen*/ + dsid.length() +
                                PUBLIC_KEY_LENGTH + SALT_LENGTH;
}

void HandshakeF0Message::write_dynamic_data(uint8_t* data) const {
  (*data++) = dsa_version_major;
  (*data++) = dsa_version_minor;
  data += write_str_with_len(data, dsid);
  data = std::copy(public_key.begin(), public_key.end(), data);
  data = std::copy(salt.begin(), salt.end(), data);
  //(*data++) = static_cast<uint8_t>(security_preference);
}

void HandshakeF0Message::parse_dynamic_headers(
    const uint8_t* data, size_t size) throw(const MessageParsingError&) {
  dsa_version_major = *data++;
  dsa_version_minor = *data++;

  data += read_str_with_len(data, dsid);

  public_key.assign(data, data + PUBLIC_KEY_LENGTH);
  data += PUBLIC_KEY_LENGTH;
  salt.assign(data, data + SALT_LENGTH);

  //security_preference = static_cast<bool>(*data++);
}

}  // namespace dsa
