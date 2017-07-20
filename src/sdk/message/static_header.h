#ifndef DSA_SDK_STATIC_HEADER_H
#define DSA_SDK_STATIC_HEADER_H

#include <cstddef>
#include <cstdint>
namespace dsa {

class StaticHeaders {
 private:
  uint32_t _message_size;
  uint16_t _header_size;
  uint8_t _type;
  uint32_t _request_id;
  uint32_t _ack_id;

 public:
  enum : size_t {
    MessageSizeOffset = 0,
    HeaderSizeOffset = MessageSizeOffset + sizeof(uint32_t),
    TypeOffset = HeaderSizeOffset + sizeof(uint16_t),
    RequestIdOffset = TypeOffset + sizeof(uint8_t),
    AckIdOffset = RequestIdOffset + sizeof(uint32_t),
    TotalSize = AckIdOffset + sizeof(uint32_t)
  };

  StaticHeaders(const uint8_t *data);
  StaticHeaders(uint32_t message_size, uint16_t header_size, uint8_t type,
                uint32_t request_id, uint32_t ack_id);

  const uint32_t &message_size() const { return _message_size; };
  const uint16_t &header_size() const { return _header_size; };
  const uint8_t &type() const { return _type; };
  const uint32_t &request_id() const { return _request_id; };
  const uint32_t &ack_id() const { return _ack_id; };
  void write(uint8_t *data);
};

}  // namespace dsa

#endif  // DSA_SDK_STATIC_HEADER_H
