#include "static_header.h"

#include <cstring>

namespace dsa {

StaticHeader::StaticHeader(const uint8_t *data) {
  std::memcpy(&_message_size, &data[message_size_offset], sizeof(_message_size));
  std::memcpy(&_header_size, &data[header_size_offset], sizeof(_header_size));
  std::memcpy(&_type, &data[type_offset], sizeof(_type));
  std::memcpy(&_request_id, &data[request_id_offset], sizeof(_request_id));
  std::memcpy(&_ack_id, &data[ack_id_offset], sizeof(_ack_id));
}

StaticHeader::StaticHeader(uint32_t message_size,
                           uint16_t header_size,
                           uint8_t type,
                           uint32_t request_id,
                           uint32_t ack_id)
    : _message_size(message_size), _header_size(header_size), _request_id(request_id), _type(type), _ack_id(ack_id) {}

const uint32_t &StaticHeader::message_size() const { return _message_size; }
const uint16_t &StaticHeader::header_size() const { return _header_size; }
const uint8_t &StaticHeader::type() const { return _type; }
const uint32_t &StaticHeader::request_id() const { return _request_id; }
const uint32_t &StaticHeader::ack_id() const { return _ack_id; }

void StaticHeader::write(uint8_t *data) {
  std::memcpy(&data[message_size_offset], &_message_size, sizeof(_message_size));
  std::memcpy(&data[header_size_offset], &_header_size, sizeof(_header_size));
  std::memcpy(&data[type_offset], &_type, sizeof(_type));
  std::memcpy(&data[request_id_offset], &_request_id, sizeof(_request_id));
  std::memcpy(&data[ack_id_offset], &_ack_id, sizeof(_ack_id));
}

}  // namespace dsa