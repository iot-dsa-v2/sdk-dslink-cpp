#include "subscribe_response_message.h"

namespace dsa {
SubscribeResponseMessage::SubscribeResponseMessage(const SharedBuffer& buffer)
    : ResponseMessage(buffer) {}
void SubscribeResponseMessage::parseDynamicHeaders() {}
}  // namespace dsa
