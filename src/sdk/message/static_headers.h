#ifndef DSA_SDK_STATIC_HEADER_H
#define DSA_SDK_STATIC_HEADER_H

#include <cstddef>
#include "enums.h"

namespace dsa {

struct StaticHeaders {
  enum : size_t {
    MESSAGE_SIZE_OFFSET = 0,
    HEADER_SIZE_OFFSET = MESSAGE_SIZE_OFFSET + sizeof(uint32_t),
    TYPE_OFFSET = HEADER_SIZE_OFFSET + sizeof(uint16_t),
    REQUEST_ID_OFFSET = TYPE_OFFSET + sizeof(uint8_t),
    ACK_ID_OFFSET = REQUEST_ID_OFFSET + sizeof(uint32_t),
    TOTAL_SIZE = ACK_ID_OFFSET + sizeof(uint32_t)
  };

  uint32_t message_size;
  uint16_t header_size;
  MessageType type;
  uint32_t request_id;
  uint32_t ack_id;

  StaticHeaders(const uint8_t *data);
  StaticHeaders(uint32_t message_size, uint16_t header_size, MessageType type,
                uint32_t request_id, uint32_t ack_id);

  void write(uint8_t *data) const;
};

}  // namespace dsa

#endif  // DSA_SDK_STATIC_HEADER_H
