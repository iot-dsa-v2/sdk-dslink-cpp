#ifndef DSA_SDK_LIST_RESPONSE_MESSAGE_H_
#define DSA_SDK_LIST_RESPONSE_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {

class ListResponseMessage : public ResponseMessage {
 public:
  ListResponseMessage(const SharedBuffer& buffer);
  void parseDynamicHeaders();
};

}  // namespace dsa

#endif  // DSA_SDK_LIST_RESPONSE_MESSAGE_H_
