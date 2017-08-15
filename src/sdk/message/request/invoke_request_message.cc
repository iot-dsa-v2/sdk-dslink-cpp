#include "dsa_common.h"

#include "invoke_request_message.h"

namespace dsa {

InvokeRequestMessage::InvokeRequestMessage(const uint8_t* begin,
                                           const uint8_t* end)
    : RequestMessage(begin, end) {
  parse_dynamic_headers(begin + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

InvokeRequestMessage::InvokeRequestMessage()
    : RequestMessage(MessageType::InvokeRequest) {}

}  // namespace dsa
