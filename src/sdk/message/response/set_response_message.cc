#include "set_response_message.h"

namespace dsa {
SetResponseMessage::SetResponseMessage(const SharedBuffer& buffer)
    : ResponseMessage(buffer) {
  parse_dynamic_headers(buffer.data + StaticHeaders::TotalSize,
                      static_headers.header_size - StaticHeaders::TotalSize);
}

}  // namespace dsa
