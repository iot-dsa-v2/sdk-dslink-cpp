#ifndef DSA_SDK_LIST_REQUEST_MESSAGE_H_
#define DSA_SDK_LIST_REQUEST_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {

class ListRequestMessage : public RequestMessage {
 public:
  ListRequestMessage(const SharedBuffer& buffer);
};

}  // namespace dsa

#endif  // DSA_SDK_LIST_REQUEST_MESSAGE_H_
