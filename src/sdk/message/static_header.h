#ifndef DSA_SDK_STATIC_HEADER_H
#define DSA_SDK_STATIC_HEADER_H

#include <cstddef>
#include <cstdint>
namespace dsa {

struct StaticHeaders {
  enum : size_t {
    MessageSizeOffset = 0,
    HeaderSizeOffset = MessageSizeOffset + sizeof(uint32_t),
    TypeOffset = HeaderSizeOffset + sizeof(uint16_t),
    RequestIdOffset = TypeOffset + sizeof(uint8_t),
    AckIdOffset = RequestIdOffset + sizeof(uint32_t),
    TotalSize = AckIdOffset + sizeof(uint32_t)
  };

  uint32_t message_size;
  uint16_t header_size;
  uint8_t type;
  uint32_t request_id;
  uint32_t ack_id;

  StaticHeaders(const uint8_t *data);
  StaticHeaders(uint32_t message_size, uint16_t header_size, uint8_t type,
                uint32_t request_id, uint32_t ack_id);

  void write(uint8_t *data) const;
};

}  // namespace dsa

#endif  // DSA_SDK_STATIC_HEADER_H
