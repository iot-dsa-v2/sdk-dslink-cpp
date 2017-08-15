#include "dsa_common.h"

#include "set_response_message.h"

namespace dsa {
SetResponseMessage::SetResponseMessage(const uint8_t* begin, const uint8_t* end)
    : ResponseMessage(begin, end) {
  parse_dynamic_headers(begin + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

SetResponseMessage::SetResponseMessage()
    : ResponseMessage(MessageType::SetResponse) {}

}  // namespace dsa
