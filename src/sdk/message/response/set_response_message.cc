#include "set_response_message.h"

namespace dsa {
SetResponseMessage::SetResponseMessage(const SharedBuffer& buffer)
    : ResponseMessage(buffer) {}
void SetResponseMessage::parseDynamicHeaders(const uint8_t* data, size_t size) {}
}  // namespace dsa
