#include "dsa_common.h"

#include "set_request_message.h"

namespace dsa {
SetRequestMessage::SetRequestMessage(const uint8_t* data, size_t size)
    : RequestMessage(data, size) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
}

SetRequestMessage::SetRequestMessage()
    : RequestMessage(MessageType::SET_REQUEST) {}
}  // namespace dsa
