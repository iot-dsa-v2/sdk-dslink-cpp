#include "dsa_common.h"

#include "invoke_request_message.h"

namespace dsa {

InvokeRequestMessage::InvokeRequestMessage(const uint8_t* data, size_t size)
    : RequestMessage(data, size) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
}

InvokeRequestMessage::InvokeRequestMessage()
    : RequestMessage(MessageType::INVOKE_REQUEST) {}

InvokeOptions InvokeRequestMessage::get_invoke_options() const { return {}; }

}  // namespace dsa
