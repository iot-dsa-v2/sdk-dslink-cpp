#include "dsa_common.h"

#include "invoke_response_message.h"

namespace dsa {
InvokeResponseMessage::InvokeResponseMessage(const uint8_t* data, size_t size)
    : ResponseMessage(data, size) {
  parse_dynamic_headers(data + StaticHeaders::TOTAL_SIZE,
                        static_headers.header_size - StaticHeaders::TOTAL_SIZE);
}

InvokeResponseMessage::InvokeResponseMessage()
    : ResponseMessage(MessageType::INVOKE_RESPONSE) {}

}  // namespace dsa
