#include "dsa_common.h"

#include "set_request_message.h"

namespace dsa {
SetRequestMessage::SetRequestMessage(const uint8_t* begin, const uint8_t* end)
    : RequestMessage(begin, end) {
  parse_dynamic_headers(begin + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

SetRequestMessage::SetRequestMessage()
    : RequestMessage(MessageType::SetRequest) {}

}  // namespace dsa
