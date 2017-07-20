#include "set_request_message.h"

namespace dsa {
SetRequestMessage::SetRequestMessage(const SharedBuffer& buffer)
    : RequestMessage(buffer) {}
void SetRequestMessage::parseDynamicHeaders(const uint8_t* data, size_t size) {}
}  // namespace dsa
