#include "subscribe_response_message.h"

namespace dsa {
SubscribeResponseMessage::SubscribeResponseMessage(const SharedBuffer& buffer)
    : ResponseMessage(buffer) {}
void SubscribeResponseMessage::parseDynamicHeaders(const uint8_t* data,
                                                   size_t size) {
  while (size > 0) {
    DynamicHeader* header = DynamicHeader::parse(data, size);
    switch (header->key()) {
      case DynamicHeader::Qos:
        break;
      case DynamicHeader::QueueSize:
        break;
      case DynamicHeader::QueueTime:
        break;
    }
  }
}
}  // namespace dsa
