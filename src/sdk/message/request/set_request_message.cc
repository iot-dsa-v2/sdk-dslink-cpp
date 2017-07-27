#include "dsa_common.h"

#include "set_request_message.h"

namespace dsa {
SetRequestMessage::SetRequestMessage(const SharedBuffer& buffer)
    : RequestMessage(buffer) {
  parse_dynamic_headers(buffer.data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

SetRequestMessage::SetRequestMessage()
    : RequestMessage(MessageType::SetRequest) {}

}  // namespace dsa
