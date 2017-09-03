#include "dsa_common.h"

#include "set_response_message.h"

namespace dsa {
SetResponseMessage::SetResponseMessage(const uint8_t* data, size_t size)
    : ResponseMessage(data, size) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
}

SetResponseMessage::SetResponseMessage()
    : ResponseMessage(MessageType::SET_RESPONSE) {}

}  // namespace dsa
