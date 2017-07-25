#include "subscribe_request_message.h"

namespace dsa {
SubscribeRequestMessage::SubscribeRequestMessage(const SharedBuffer& buffer)
    : RequestMessage(buffer) {
  parse_dynamic_headers(buffer.data + StaticHeaders::TotalSize,
                      static_headers.header_size - StaticHeaders::TotalSize);
}

}  // namespace dsa
