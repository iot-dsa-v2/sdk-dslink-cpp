#include "invoke_response_message.h"

namespace dsa {
InvokeResponseMessage::InvokeResponseMessage(const SharedBuffer& buffer)
    : ResponseMessage(buffer) {}
void InvokeResponseMessage::parseDynamicHeaders() {}
}  // namespace dsa
