#include "invoke_response_message.h"

namespace dsa {
InvokeResponseMessage::InvokeResponseMessage(const SharedBuffer& buffer)
    : ResponseMessage(buffer) {
  parseDynamicHeaders(buffer.data + StaticHeaders::TotalSize,
                      static_headers.header_size() - StaticHeaders::TotalSize);
}
void InvokeResponseMessage::parseDynamicHeaders(const uint8_t* data,
                                                size_t size) {}
}  // namespace dsa
