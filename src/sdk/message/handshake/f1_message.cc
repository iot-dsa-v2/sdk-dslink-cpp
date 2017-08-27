#include "dsa_common.h"

#include "f1_message.h"

#include "util/little_endian.h"

namespace dsa {

HandshakeF1Message::HandshakeF1Message(const uint8_t* data, size_t size)
    : Message(data, size), public_key(PublicKeyLength), salt(SaltLength) {
  parse_dynamic_headers(data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

HandshakeF1Message::HandshakeF1Message()
    : Message(MessageType::Handshake1),
      public_key(PublicKeyLength),
      salt(SaltLength) {}

void HandshakeF1Message::update_static_header() {
  static_headers.header_size = (uint16_t)StaticHeaders::TotalSize;
  static_headers.message_size = StaticHeaders::TotalSize + 2 /*dsidlen*/ +
                                dsid.length() + PublicKeyLength + SaltLength;
}

void HandshakeF1Message::write_dynamic_data(uint8_t* data) const {
  data += write_16_t(data, static_cast<uint16_t>(dsid.length()));
  data += dsid.copy(reinterpret_cast<char*>(data), dsid.length());

  data = std::copy(public_key.begin(), public_key.end(), data);
  data = std::copy(salt.begin(), salt.end(), data);
}

void HandshakeF1Message::parse_dynamic_headers(
    const uint8_t* data, size_t size) throw(const MessageParsingError&) {
  uint16_t dsid_length = read_16_t(data);

  data += dsid.assign(reinterpret_cast<const char*>(data), dsid_length).size();
  public_key.assign(data, data + PublicKeyLength);
  data += public_key.size();

  salt.assign(data, data + SaltLength);
}

}  // namespace dsa
