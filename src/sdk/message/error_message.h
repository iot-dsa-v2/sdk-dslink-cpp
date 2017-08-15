#ifndef DSA_SDK_ERROR_MESSAGE_H
#define DSA_SDK_ERROR_MESSAGE_H

#include "base_message.h"

namespace dsa {


class ErrorMessage :public ResponseMessage {
 public:
  ErrorMessage(MessageType type, MessageStatus status);
};
}
#endif //DSA_SDK_ERROR_MESSAGE_H
