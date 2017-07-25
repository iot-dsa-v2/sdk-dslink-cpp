#include "list_request_message.h"

namespace dsa {
ListRequestMessage::ListRequestMessage(const SharedBuffer& buffer)
    : RequestMessage(buffer) {
  parse_dynamic_headers(buffer.data + StaticHeaders::TotalSize,
                      static_headers.header_size - StaticHeaders::TotalSize);
}

}  // namespace dsa
