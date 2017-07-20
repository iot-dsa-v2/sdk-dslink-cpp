#ifndef DSA_SDK_SET_REQUEST_MESSAGE_H_
#define DSA_SDK_SET_REQUEST_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {

class SetRequestMessage : public RequestMessage {
 public:
  SetRequestMessage(const SharedBuffer& buffer);
};

}  // namespace dsa

#endif  // DSA_SDK_SET_REQUEST_MESSAGE_H_
