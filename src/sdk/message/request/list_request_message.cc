#include "dsa_common.h"

#include "list_request_message.h"

namespace dsa {
ListRequestMessage::ListRequestMessage(const uint8_t* data, size_t size)
    : RequestMessage(data, size) {
  parse_dynamic_headers(data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}
ListRequestMessage::ListRequestMessage()
    : RequestMessage(MessageType::ListRequest) {}
}  // namespace dsa
