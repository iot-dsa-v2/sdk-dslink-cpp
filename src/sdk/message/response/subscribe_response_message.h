#ifndef DSA_SDK_SUBSCRIBE_RESPONSE_MESSAGE_H_
#define DSA_SDK_SUBSCRIBE_RESPONSE_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {

class SubscribeResponseMessage : public ResponseMessage {
 public:
  SubscribeResponseMessage(const SharedBuffer& buffer);

 protected:
  void parse_dynamic_headers(const uint8_t* data, size_t size);
};

}  // namespace dsa

#endif  // DSA_SDK_SUBSCRIBE_RESPONSE_MESSAGE_H_
