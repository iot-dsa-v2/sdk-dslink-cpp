#ifndef DSA_SDK_STATIC_HEADER_H
#define DSA_SDK_STATIC_HEADER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <cstddef>
#include "util/enums.h"

namespace dsa {

struct StaticHeaders {
  enum : size_t {
    MESSAGE_SIZE_OFFSET = 0,
    HEADER_SIZE_OFFSET = MESSAGE_SIZE_OFFSET + sizeof(uint32_t),
    TYPE_OFFSET = HEADER_SIZE_OFFSET + sizeof(uint16_t),
    REQUEST_ID_OFFSET = TYPE_OFFSET + sizeof(uint8_t),
    ACK_ID_OFFSET = REQUEST_ID_OFFSET + sizeof(uint32_t)

  };
  static const size_t TOTAL_SIZE = ACK_ID_OFFSET + sizeof(uint32_t);
  static const size_t SHORT_TOTAL_SIZE = REQUEST_ID_OFFSET;

  int32_t message_size;
  uint16_t header_size;
  MessageType type;
  int32_t rid;
  int32_t ack_id;

  StaticHeaders(const uint8_t *data);
  StaticHeaders(int32_t message_size, uint16_t header_size, MessageType type,
                int32_t rid, int32_t ack_id);

  void write(uint8_t *data) const;

  uint8_t *write(uint8_t *data, int32_t rid, int32_t ack_id) const;
};

}  // namespace dsa

#endif  // DSA_SDK_STATIC_HEADER_H
