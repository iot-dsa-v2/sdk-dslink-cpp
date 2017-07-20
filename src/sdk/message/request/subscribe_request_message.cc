#include "subscribe_request_message.h"

namespace dsa {
SubscribeRequestMessage::SubscribeRequestMessage(const SharedBuffer& buffer)
    : RequestMessage(buffer) {}
void SubscribeRequestMessage::parseDynamicHeaders() {}
}  // namespace dsa
