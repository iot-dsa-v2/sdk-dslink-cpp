#include "dsa_common.h"

#include "static_headers.h"

#include "util/little_endian.h"

namespace dsa {

StaticHeaders::StaticHeaders(const uint8_t *data) {
  message_size = read_32_t(data);
  data += sizeof(int32_t);

  header_size = read_16_t(data);
  data += sizeof(uint16_t);

  type = static_cast<MessageType>(*data);

  if (static_cast<uint8_t>(type) >= 0xF0) {
    rid = 0;
    ack_id = 0;
    return;
  }

  data++;

  rid = read_32_t(data);
  data += sizeof(int32_t);

  ack_id = read_32_t(data);
  // data += sizeof(int32_t);
}

StaticHeaders::StaticHeaders(int32_t message_size, uint16_t header_size,
                             MessageType type, int32_t rid, int32_t ack_id)
    : message_size(message_size),
      header_size(header_size),
      rid(rid),
      type(type),
      ack_id(ack_id) {}

void StaticHeaders::write(uint8_t *data) const {
  data += write_32_t(data, message_size);
  data += write_16_t(data, header_size);
  *(data++) = static_cast<uint8_t>(type);

  if (static_cast<uint8_t>(type) < 0xF0) {
    data += write_32_t(data, rid);
    /* data += */ write_32_t(data, ack_id);
  }
}

uint8_t *StaticHeaders::write(uint8_t *data, int32_t rid,
                              int32_t ack_id) const {
  data += write_32_t(data, message_size);
  data += write_16_t(data, header_size);
  *(data++) = static_cast<uint8_t>(type);

  if (static_cast<uint8_t>(type) < 0xF0) {
    data += write_32_t(data, rid);
    data += write_32_t(data, ack_id);
  }

  return data;
}

}  // namespace dsa