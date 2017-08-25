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

HandshakeF3Message::HandshakeF3Message() : Message(MessageType::Handshake1) {}

// HandshakeF3Message::HandshakeF3Message(const HandshakeF3Message&){}

void HandshakeF3Message::update_static_header() {}
void HandshakeF3Message::write_dynamic_data(uint8_t* data) const {}
void HandshakeF3Message::parse_dynamic_headers(
    const uint8_t* data, size_t size) throw(const MessageParsingError&) {}

}  // namespace dsa
