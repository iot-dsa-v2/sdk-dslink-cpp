#ifndef DSA_SUBSCRIBE_INVOKE_REQUEST_MESSAGE_H_
#define DSA_SUBSCRIBE_INVOKE_REQUEST_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {

class SubscribeRequestMessage : public RequestMessage {
 public:
  SubscribeRequestMessage(const SharedBuffer& buffer);

 protected:
  void parseDynamicHeaders(const uint8_t* data, size_t size);
};

}  // namespace dsa

#endif  // DSA_SUBSCRIBE_INVOKE_REQUEST_MESSAGE_H_
