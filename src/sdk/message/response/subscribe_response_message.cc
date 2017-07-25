#include "subscribe_response_message.h"

namespace dsa {
SubscribeResponseMessage::SubscribeResponseMessage(const SharedBuffer& buffer)
    : ResponseMessage(buffer) {
  parse_dynamic_headers(buffer.data + StaticHeaders::TotalSize,
                      static_headers.header_size - StaticHeaders::TotalSize);
}


}  // namespace dsa
