#include "list_response_message.h"

namespace dsa {
ListResponseMessage::ListResponseMessage(const SharedBuffer& buffer)
    : ResponseMessage(buffer) {
  parseDynamicHeaders(buffer.data + StaticHeaders::TotalSize,
                      static_headers.header_size() - StaticHeaders::TotalSize);
}
void ListResponseMessage::parseDynamicHeaders(const uint8_t* data,
                                              size_t size) {}
}  // namespace dsa
