#include "static_header.h"

#include <cstring>

namespace dsa {

StaticHeaders::StaticHeaders(const uint8_t *data) {
  std::memcpy(&_message_size, &data[MESSAGE_SIZE_OFFSET], sizeof(_message_size));
  std::memcpy(&_header_size, &data[HEADER_SIZE_OFFSET], sizeof(_header_size));
  std::memcpy(&_type, &data[TYPE_OFFSET], sizeof(_type));
  std::memcpy(&_request_id, &data[REQUEST_ID_OFFSET], sizeof(_request_id));
  std::memcpy(&_ack_id, &data[ACK_ID_OFFSET], sizeof(_ack_id));
}

StaticHeaders::StaticHeaders(uint32_t message_size,
                           uint16_t header_size,
                           uint8_t type,
                           uint32_t request_id,
                           uint32_t ack_id)
    : _message_size(message_size), _header_size(header_size), _request_id(request_id), _type(type), _ack_id(ack_id) {}

const uint32_t &StaticHeaders::message_size() const { return _message_size; }
const uint16_t &StaticHeaders::header_size() const { return _header_size; }
const uint8_t &StaticHeaders::type() const { return _type; }
const uint32_t &StaticHeaders::request_id() const { return _request_id; }
const uint32_t &StaticHeaders::ack_id() const { return _ack_id; }

void StaticHeaders::write(uint8_t *data) {
  std::memcpy(&data[MESSAGE_SIZE_OFFSET], &_message_size, sizeof(_message_size));
  std::memcpy(&data[HEADER_SIZE_OFFSET], &_header_size, sizeof(_header_size));
  std::memcpy(&data[TYPE_OFFSET], &_type, sizeof(_type));
  std::memcpy(&data[REQUEST_ID_OFFSET], &_request_id, sizeof(_request_id));
  std::memcpy(&data[ACK_ID_OFFSET], &_ack_id, sizeof(_ack_id));
}

}  // namespace dsa