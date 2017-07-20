#include "base_message.h"

namespace dsa {
Message::Message(const SharedBuffer& buffer) : static_headers(buffer.data) {
  parseDynamicHeaders(buffer.data + StaticHeaders::TotalSize,
                      static_headers.header_size() - StaticHeaders::TotalSize);
};
RequestMessage::RequestMessage(const SharedBuffer& buffer) : Message(buffer){};
ResponseMessage::ResponseMessage(const SharedBuffer& buffer)
    : Message(buffer){};
}  // namespace dsa
