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

HandshakeF0Message::HandshakeF0Message() : Message(MessageType::Handshake1) {}

// HandshakeF0Message::HandshakeF0Message(const HandshakeF0Message&){}

void HandshakeF0Message::update_static_header() {}
void HandshakeF0Message::write_dynamic_data(uint8_t* data) const {}
void HandshakeF0Message::parse_dynamic_headers(
    const uint8_t* data, size_t size) throw(const MessageParsingError&) {}

}  // namespace dsa
