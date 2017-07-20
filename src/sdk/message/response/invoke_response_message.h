#ifndef DSA_SDK_INVOKE_RESPONSE_MESSAGE_H_
#define DSA_SDK_INVOKE_RESPONSE_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {

class InvokeResponseMessage : public ResponseMessage {
 public:
  InvokeResponseMessage(const SharedBuffer& buffer);
};

}  // namespace dsa

#endif  // DSA_SDK_INVOKE_RESPONSE_MESSAGE_H_
