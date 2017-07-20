#include "list_request_message.h"

namespace dsa {
ListRequestMessage::ListRequestMessage(const SharedBuffer& buffer)
    : RequestMessage(buffer) {}
void ListRequestMessage::parseDynamicHeaders() {}
}  // namespace dsa
