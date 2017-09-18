#include "dsa_common.h"

#include "ack_message.h"

#include "util/little_endian.h"

namespace dsa {

AckMessage::AckMessage(const uint8_t* data, size_t size)
    : Message(data, size), _ack(read_32_t(data + StaticHeaders::TOTAL_SIZE)) {}

AckMessage::AckMessage() : Message(MessageType::ACK) {}

void AckMessage::write_dynamic_data(uint8_t* data) const {
  write_32_t(data, _ack);
}

void AckMessage::update_static_header() {
  uint32_t header_size = StaticHeaders::TOTAL_SIZE;

  static_headers.message_size = StaticHeaders::TOTAL_SIZE + sizeof(int32_t);
  static_headers.header_size = StaticHeaders::TOTAL_SIZE;
}
}