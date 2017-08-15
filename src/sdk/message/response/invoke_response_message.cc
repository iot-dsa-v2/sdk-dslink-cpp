#include "dsa_common.h"

#include "invoke_response_message.h"

namespace dsa {
InvokeResponseMessage::InvokeResponseMessage(const uint8_t* begin, const uint8_t* end)
    : ResponseMessage(begin, end) {
  parse_dynamic_headers(begin + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

InvokeResponseMessage::InvokeResponseMessage()
    : ResponseMessage(MessageType::InvokeResponse) {}

}  // namespace dsa
