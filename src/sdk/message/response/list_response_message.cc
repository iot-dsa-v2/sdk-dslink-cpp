#include "list_response_message.h"

namespace dsa {
ListResponseMessage::ListResponseMessage(const SharedBuffer& buffer)
    : ResponseMessage(buffer) {
  parse_dynamic_headers(buffer.data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

ListResponseMessage::ListResponseMessage()
    : ResponseMessage(MessageType::ListResponse) {}

}  // namespace dsa
