#ifndef DSA_SDK_STATIC_HEADER_H
#define DSA_SDK_STATIC_HEADER_H

#include <cstddef>
#include <cstdint>
namespace dsa {

enum MessageType : uint8_t {  // https://github.com/iot-dsa-v2/docs/blob/master/protocol/Header-Structure.md
  SubscribeRequest = 0x01,
  ListRequest = 0x02,
  InvokeRequest = 0x03,
  SetRequest = 0x04,
  SubscribeResponse = 0x81,
  ListResponse = 0x82,
  InvokeResponse = 0x83,
  SetResponse = 0x84,
  Handshake0 = 0xF0,
  Handshake1 = 0xF1,
  Handshake2 = 0xF2,
  Handshake3 = 0xF3
};

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
