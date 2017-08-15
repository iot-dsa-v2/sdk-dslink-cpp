#include "dsa_common.h"

#include "invoke_request_message.h"

namespace dsa {

InvokeRequestMessage::InvokeRequestMessage(const uint8_t* data, size_t size)
    : RequestMessage(data, size) {
  parse_dynamic_headers(data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

InvokeRequestMessage::InvokeRequestMessage()
    : RequestMessage(MessageType::InvokeRequest) {}

InvokeOptions InvokeRequestMessage::get_invoke_options() const {
  return {};
}

}  // namespace dsa
