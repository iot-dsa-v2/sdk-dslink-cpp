#include "dsa_common.h"

#include "ping_message.h"

#include "util/little_endian.h"

namespace dsa {

PingMessage::PingMessage(const uint8_t* data, size_t size)
    : Message(data, size) {}

PingMessage::PingMessage() : Message(MessageType::PING) {}

void PingMessage::update_static_header() {
  static_headers.message_size = StaticHeaders::SHORT_TOTAL_SIZE;
  static_headers.header_size = StaticHeaders::SHORT_TOTAL_SIZE;
}
}
