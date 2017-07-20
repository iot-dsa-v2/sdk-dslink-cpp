#include "invoke_response_message.h"

namespace dsa {
InvokeResponseMessage::InvokeResponseMessage(const SharedBuffer& buffer)
    : ResponseMessage(buffer) {}
void InvokeResponseMessage::parseDynamicHeaders(const uint8_t* data, size_t size) {}
}  // namespace dsa
