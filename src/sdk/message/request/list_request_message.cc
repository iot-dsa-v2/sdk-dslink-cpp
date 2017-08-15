#include "dsa_common.h"

#include "list_request_message.h"

namespace dsa {
ListRequestMessage::ListRequestMessage(const uint8_t* begin, const uint8_t* end)
    : RequestMessage(begin, end) {
  parse_dynamic_headers(begin + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}
ListRequestMessage::ListRequestMessage()
    : RequestMessage(MessageType::ListRequest) {}
}  // namespace dsa
