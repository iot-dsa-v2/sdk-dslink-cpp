#include "dsa_common.h"
#include "error_message.h"

namespace dsa {
ErrorMessage::ErrorMessage(MessageType type, MessageStatus s, uint32_t request_id)
    : ResponseMessage(type) {
  status.reset(new DynamicByteHeader(DynamicHeader::STATUS, static_cast<uint8_t>(s)));
  static_headers.request_id = request_id;
}

void ErrorMessage::write_dynamic_data(uint8_t *data) const {
  if (status != nullptr) {
    status->write(data);
//    data += status->size();
  }
}

void ErrorMessage::update_static_header() {
  uint32_t header_size = StaticHeaders::TotalSize;
  if (status != nullptr) {
    header_size += status->size();
  }

  static_headers.message_size = header_size;
  static_headers.header_size = (uint16_t)header_size;
}
}
