#include "dsa_common.h"

#include "static_header.h"

#include <cstring>

namespace dsa {

StaticHeaders::StaticHeaders(const uint8_t *data) {
  std::memcpy(&message_size, &data[MessageSizeOffset], sizeof(message_size));
  std::memcpy(&header_size, &data[HeaderSizeOffset], sizeof(header_size));
  std::memcpy(&type, &data[TypeOffset], sizeof(type));
  std::memcpy(&request_id, &data[RequestIdOffset], sizeof(request_id));
  std::memcpy(&ack_id, &data[AckIdOffset], sizeof(ack_id));
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
  std::memcpy(&data[MessageSizeOffset], &message_size, sizeof(message_size));
  std::memcpy(&data[HeaderSizeOffset], &header_size, sizeof(header_size));
  std::memcpy(&data[TypeOffset], &type, sizeof(type));
  std::memcpy(&data[RequestIdOffset], &request_id, sizeof(request_id));
  std::memcpy(&data[AckIdOffset], &ack_id, sizeof(ack_id));
}

}  // namespace dsa