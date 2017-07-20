#include "list_response_message.h"

namespace dsa {
ListResponseMessage::ListResponseMessage(const SharedBuffer& buffer)
    : ResponseMessage(buffer) {}
void ListResponseMessage::parseDynamicHeaders() {}
}  // namespace dsa
