#include "set_request_message.h"

namespace dsa {
SetRequestMessage::SetRequestMessage(const SharedBuffer& buffer)
    : RequestMessage(buffer) {
  parseDynamicHeaders(buffer.data + StaticHeaders::TotalSize,
                      static_headers.header_size() - StaticHeaders::TotalSize);
}
void SetRequestMessage::parseDynamicHeaders(const uint8_t* data, size_t size) {}
}  // namespace dsa
