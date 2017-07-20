#ifndef DSA_SDK_INVOKE_RESPONSE_MESSAGE_H_
#define DSA_SDK_INVOKE_RESPONSE_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {

class InvokeResponseMessage : public ResponseMessage {
 public:
  InvokeResponseMessage(const SharedBuffer& buffer);

 protected:
  void parseDynamicHeaders(const uint8_t* data, size_t size);
};

}  // namespace dsa

#endif  // DSA_SDK_INVOKE_RESPONSE_MESSAGE_H_
