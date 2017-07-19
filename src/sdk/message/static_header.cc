#include "static_header.h"

#include <cstring>

namespace dsa {

StaticHeaders::StaticHeaders(const uint8_t *data) {
  std::memcpy(&_message_size, &data[MessageSizeOffset], sizeof(_message_size));
  std::memcpy(&_header_size, &data[HeaderSizeOffset], sizeof(_header_size));
  std::memcpy(&_type, &data[TypeOffset], sizeof(_type));
  std::memcpy(&_request_id, &data[RequestIdOffset], sizeof(_request_id));
  std::memcpy(&_ack_id, &data[AckIdOffset], sizeof(_ack_id));
}

StaticHeaders::StaticHeaders(uint32_t message_size,
                           uint16_t header_size,
                           uint8_t type,
                           uint32_t request_id,
                           uint32_t ack_id)
    : _message_size(message_size), _header_size(header_size), _request_id(request_id), _type(type), _ack_id(ack_id) {}

void StaticHeaders::write(uint8_t *data) {
  std::memcpy(&data[MessageSizeOffset], &_message_size, sizeof(_message_size));
  std::memcpy(&data[HeaderSizeOffset], &_header_size, sizeof(_header_size));
  std::memcpy(&data[TypeOffset], &_type, sizeof(_type));
  std::memcpy(&data[RequestIdOffset], &_request_id, sizeof(_request_id));
  std::memcpy(&data[AckIdOffset], &_ack_id, sizeof(_ack_id));
}

}  // namespace dsa