#ifndef DSA_SDK_ERROR_MESSAGE_H
#define DSA_SDK_ERROR_MESSAGE_H

#include "base_message.h"

namespace dsa {


class ErrorMessage :public ResponseMessage {
 public:
  ErrorMessage(MessageType type, MessageStatus status, uint32_t rid = 0);
  void write_dynamic_data(uint8_t* data) const override;
  void update_static_header() override;
};
}
#endif //DSA_SDK_ERROR_MESSAGE_H
