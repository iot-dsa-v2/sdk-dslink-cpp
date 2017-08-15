#include "dsa_common.h"
#include "error_message.h"

namespace dsa {
ErrorMessage::ErrorMessage(MessageType type, MessageStatus s)
    : ResponseMessage(type) {
  status.reset(
      new DynamicByteHeader(DynamicHeader::Status, static_cast<uint8_t>(s)));
}

}
