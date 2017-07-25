#include "base_message.h"

namespace dsa {
Message::Message(const SharedBuffer& buffer) : static_headers(buffer.data){};

void Message::write(uint8_t* data) const {
  static_headers.write(data);
  write_dynamic_data(data + StaticHeaders::TotalSize);
}

RequestMessage::RequestMessage(const SharedBuffer& buffer) : Message(buffer){};
ResponseMessage::ResponseMessage(const SharedBuffer& buffer)
    : Message(buffer){};
}  // namespace dsa
