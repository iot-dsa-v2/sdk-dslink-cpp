#include "dsa_common.h"

#include "list_request_message.h"

namespace dsa {
ListRequestMessage::ListRequestMessage(const SharedBuffer& buffer)
    : RequestMessage(buffer) {
  parse_dynamic_headers(buffer.data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}
ListRequestMessage::ListRequestMessage()
    : RequestMessage(MessageType::ListRequest) {}

ListOptions ListRequestMessage::get_list_options() const {
  return {};
}
}  // namespace dsa
