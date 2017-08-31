#include "dsa_common.h"

#include "static_headers.h"

#include <cstring>

namespace dsa {

StaticHeaders::StaticHeaders(const uint8_t *data) {
  std::memcpy(&message_size, &data[MESSAGE_SIZE_OFFSET], sizeof(message_size));
  std::memcpy(&header_size, &data[HEADER_SIZE_OFFSET], sizeof(header_size));
  std::memcpy(&type, &data[TYPE_OFFSET], sizeof(type));
  std::memcpy(&request_id, &data[REQUEST_ID_OFFSET], sizeof(request_id));
  std::memcpy(&ack_id, &data[ACK_ID_OFFSET], sizeof(ack_id));
}

StaticHeaders::StaticHeaders(uint32_t message_size, uint16_t header_size,
                             MessageType type, uint32_t request_id,
                             uint32_t ack_id)
    : message_size(message_size),
      header_size(header_size),
      request_id(request_id),
      type(type),
      ack_id(ack_id) {}

void StaticHeaders::write(uint8_t *data) const {
  std::memcpy(&data[MESSAGE_SIZE_OFFSET], &message_size, sizeof(message_size));
  std::memcpy(&data[HEADER_SIZE_OFFSET], &header_size, sizeof(header_size));
  std::memcpy(&data[TYPE_OFFSET], &type, sizeof(type));
  std::memcpy(&data[REQUEST_ID_OFFSET], &request_id, sizeof(request_id));
  std::memcpy(&data[ACK_ID_OFFSET], &ack_id, sizeof(ack_id));
}

}  // namespace dsa