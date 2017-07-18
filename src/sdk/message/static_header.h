#ifndef DSA_SDK_STATIC_HEADER_H
#define DSA_SDK_STATIC_HEADER_H

#include <cstdint>
namespace dsa {

class StaticHeader {
 private:
  enum {
    message_size_offset = 0,
    header_size_offset = message_size_offset + sizeof(uint32_t),
    type_offset = header_size_offset + sizeof(uint16_t),
    request_id_offset = type_offset + sizeof(uint8_t),
    ack_id_offset = request_id_offset + sizeof(uint32_t)
  };

  uint32_t _message_size;
  uint16_t _header_size;
  uint8_t _type;
  uint32_t _request_id;
  uint32_t _ack_id;
 public:
  StaticHeader(const uint8_t * data);
  const uint32_t &message_size() const;
  const uint16_t &header_size() const;
  const uint8_t &type() const;
  const uint32_t &request_id() const;
  const uint32_t &ack_id() const;
};

}  // namespace dsa

#endif //DSA_SDK_STATIC_HEADER_H
