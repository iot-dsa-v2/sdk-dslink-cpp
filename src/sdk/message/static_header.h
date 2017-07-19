#ifndef DSA_SDK_STATIC_HEADER_H
#define DSA_SDK_STATIC_HEADER_H

#include <cstdint>
#include <cstddef>
namespace dsa {

class StaticHeaders {
 public:
  static const size_t MESSAGE_SIZE_OFFSET = 0;
  static const size_t HEADER_SIZE_OFFSET =
      MESSAGE_SIZE_OFFSET + sizeof(uint32_t);
  static const size_t TYPE_OFFSET = HEADER_SIZE_OFFSET + sizeof(uint16_t);
  static const size_t REQUEST_ID_OFFSET = TYPE_OFFSET + sizeof(uint8_t);
  static const size_t ACK_ID_OFFSET = REQUEST_ID_OFFSET + sizeof(uint32_t);
  static const size_t TOTAL_SIZE = ACK_ID_OFFSET + sizeof(uint32_t);

 private:
  uint32_t _message_size;
  uint16_t _header_size;
  uint8_t _type;
  uint32_t _request_id;
  uint32_t _ack_id;

 public:
  StaticHeaders(const uint8_t *data);
  StaticHeaders(uint32_t message_size, uint16_t header_size, uint8_t type,
                uint32_t request_id, uint32_t ack_id);

  const uint32_t &message_size() const;
  const uint16_t &header_size() const;
  const uint8_t &type() const;
  const uint32_t &request_id() const;
  const uint32_t &ack_id() const;
  void write(uint8_t *data);
};

}  // namespace dsa

#endif  // DSA_SDK_STATIC_HEADER_H
