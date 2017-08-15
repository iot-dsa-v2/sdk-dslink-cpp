#include "dsa_common.h"

#include "invoke_response_message.h"

namespace dsa {
InvokeResponseMessage::InvokeResponseMessage(const uint8_t* data, size_t size)
    : ResponseMessage(data, size) {
  parse_dynamic_headers(data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

InvokeResponseMessage::InvokeResponseMessage()
    : ResponseMessage(MessageType::InvokeResponse) {}

}  // namespace dsa
