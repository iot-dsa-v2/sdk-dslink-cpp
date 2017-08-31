#include "dsa_common.h"

#include "set_request_message.h"

namespace dsa {
SetRequestMessage::SetRequestMessage(const uint8_t* data, size_t size)
    : RequestMessage(data, size) {
  parse_dynamic_headers(data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

SetRequestMessage::SetRequestMessage()
    : RequestMessage(MessageType::SET_REQUEST) {}

SetOptions SetRequestMessage::get_set_options() const {
  return {};
}
}  // namespace dsa
