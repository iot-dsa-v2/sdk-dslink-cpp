#ifndef DSA_SDK_INVOKE_REQUEST_MESSAGE_H_
#define DSA_SDK_INVOKE_REQUEST_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {

class InvokeRequestMessage : public RequestMessage {
 public:
  InvokeRequestMessage(const SharedBuffer& buffer);
};

}  // namespace dsa

#endif  // DSA_SDK_INVOKE_REQUEST_MESSAGE_H_
