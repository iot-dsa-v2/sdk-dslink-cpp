#include "dsa_common.h"

#include "list_request_message.h"

namespace dsa {
ListRequestMessage::ListRequestMessage(const uint8_t* data, size_t size)
    : RequestMessage(data, size) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
}
ListRequestMessage::ListRequestMessage()
    : RequestMessage(MessageType::LIST_REQUEST) {}

ListOptions ListRequestMessage::get_list_options() const { return {}; }
}  // namespace dsa
