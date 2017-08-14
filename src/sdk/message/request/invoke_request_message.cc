#include "dsa_common.h"

#include "invoke_request_message.h"

namespace dsa {

InvokeRequestMessage::InvokeRequestMessage(const SharedBuffer& buffer)
    : RequestMessage(buffer) {
  parse_dynamic_headers(buffer.data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

InvokeRequestMessage::InvokeRequestMessage()
    : RequestMessage(MessageType::InvokeRequest) {}

InvokeOptions InvokeRequestMessage::get_invoke_options() const {
  return {};
}

}  // namespace dsa
