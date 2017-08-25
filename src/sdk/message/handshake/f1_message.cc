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

HandshakeF1Message::HandshakeF1Message() : Message(MessageType::Handshake1) {}

// HandshakeF1Message::HandshakeF1Message(const HandshakeF1Message&){}

void HandshakeF1Message::update_static_header() {}
void HandshakeF1Message::write_dynamic_data(uint8_t* data) const {}
void HandshakeF1Message::parse_dynamic_headers(
    const uint8_t* data, size_t size) throw(const MessageParsingError&) {}

}  // namespace dsa
