#ifndef DSA_SDK_STATIC_HEADER_H
#define DSA_SDK_STATIC_HEADER_H

#include <cstdint>
namespace dsa {

class StaticHeader {
 private:
  enum {
    message_size_offset = 0,
    header_size_offset = 4,
    type_offset = 6,
    request_id_offset = 7
  };

  uint32_t _message_size;
  uint16_t _header_size;
  uint8_t _type;
  uint32_t _request_id;
 public:
  StaticHeader(uint8_t * data);
  const uint32_t &message_size() const;
  const uint16_t &header_size() const;
  const uint8_t &type() const;
  const uint32_t &request_id() const;
};

}  // namespace dsa

#endif //DSA_SDK_STATIC_HEADER_H
