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

HandshakeF2Message::HandshakeF2Message() : Message(MessageType::Handshake1) {}

// HandshakeF2Message::HandshakeF2Message(const HandshakeF2Message&){}

void HandshakeF2Message::update_static_header() {}
void HandshakeF2Message::write_dynamic_data(uint8_t* data) const {}
void HandshakeF2Message::parse_dynamic_headers(
    const uint8_t* data, size_t size) throw(const MessageParsingError&) {}

}  // namespace dsa
