#ifndef DSA_SDK_SET_RESPONSE_MESSAGE_H_
#define DSA_SDK_SET_RESPONSE_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {

class SetResponseMessage : public ResponseMessage {
 public:
  SetResponseMessage(const SharedBuffer& buffer);
};

}  // namespace dsa

#endif  // DSA_SDK_SET_RESPONSE_MESSAGE_H_
