#include "list_response_message.h"

namespace dsa {
ListResponseMessage::ListResponseMessage(const SharedBuffer& buffer)
    : ResponseMessage(buffer) {}
void ListResponseMessage::parseDynamicHeaders(const uint8_t* data, size_t size) {}
}  // namespace dsa
