#include "dsa_common.h"

#include "f1_message.h"

#include "util/little_endian.h"

namespace dsa {

HandshakeF1Message::HandshakeF1Message(const uint8_t* data, size_t size)
    : Message(data, size), public_key(PUBLIC_KEY_LENGTH), salt(SALT_LENGTH) {
  parse_dynamic_headers(data + StaticHeaders::SHORT_TOTAL_SIZE,
                        static_headers.header_size - StaticHeaders::SHORT_TOTAL_SIZE);
}

HandshakeF1Message::HandshakeF1Message()
    : Message(MessageType::HANDSHAKE1),
      public_key(PUBLIC_KEY_LENGTH),
      salt(SALT_LENGTH) {}

void HandshakeF1Message::update_static_header() {
  static_headers.header_size = (uint16_t)StaticHeaders::SHORT_TOTAL_SIZE;
  static_headers.message_size = StaticHeaders::SHORT_TOTAL_SIZE + 2 /*dsidlen*/ +
                                dsid.length() + PUBLIC_KEY_LENGTH + SALT_LENGTH;
}

void HandshakeF1Message::write_dynamic_data(uint8_t* data) const {
  data += write_str_with_len(data, dsid);
  data = std::copy(public_key.begin(), public_key.end(), data);
  data = std::copy(salt.begin(), salt.end(), data);
}

void HandshakeF1Message::parse_dynamic_headers(
    const uint8_t* data, size_t size) throw(const MessageParsingError&) {
  data += read_str_with_len(data, dsid);
  public_key.assign(data, data + PUBLIC_KEY_LENGTH);
  data += public_key.size();

  salt.assign(data, data + SALT_LENGTH);
}

}  // namespace dsa
