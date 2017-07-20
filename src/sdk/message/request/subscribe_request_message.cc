#include "subscribe_request_message.h"

namespace dsa {
SubscribeRequestMessage::SubscribeRequestMessage(const SharedBuffer& buffer)
    : RequestMessage(buffer) {}
void SubscribeRequestMessage::parseDynamicHeaders(const uint8_t* data, size_t size) {
  while (size > 0) {
    DynamicHeader * header = DynamicHeader::parse(data, size);

  }
}
}  // namespace dsa
