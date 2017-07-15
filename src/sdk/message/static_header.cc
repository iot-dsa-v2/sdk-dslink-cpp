#include "static_header.h"

#include <cstring>

namespace dsa {

StaticHeader::StaticHeader(uint8_t *data) {
  std::memcpy(&_message_size, &data[message_size_offset], sizeof(_message_size));
  std::memcpy(&_header_size, &data[header_size_offset], sizeof(_header_size));
  std::memcpy(&_type, &data[type_offset], sizeof(_type));
  std::memcpy(&_request_id, &data[request_id_offset], sizeof(_request_id));
}

}  // namespace dsa