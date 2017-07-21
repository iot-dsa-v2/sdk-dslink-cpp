#include "base_message.h"

namespace dsa {
Message::Message(const SharedBuffer& buffer) : static_headers(buffer.data) {

};
RequestMessage::RequestMessage(const SharedBuffer& buffer) : Message(buffer){};
ResponseMessage::ResponseMessage(const SharedBuffer& buffer)
    : Message(buffer){};
}  // namespace dsa
