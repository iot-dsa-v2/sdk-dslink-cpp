#include "invoke_request_message.h"

namespace dsa {

InvokeRequestMessage::InvokeRequestMessage(const SharedBuffer& buffer)
    : RequestMessage(buffer) {}

void InvokeRequestMessage::parseDynamicHeaders() {}

}  // namespace dsa
