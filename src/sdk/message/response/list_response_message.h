#ifndef DSA_SDK_LIST_RESPONSE_MESSAGE_H_
#define DSA_SDK_LIST_RESPONSE_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {

class ListResponseMessage : public ResponseMessage {
 public:
  ListResponseMessage(const SharedBuffer& buffer);

 protected:
  void parseDynamicHeaders(const uint8_t* data, size_t size);
};

}  // namespace dsa

#endif  // DSA_SDK_LIST_RESPONSE_MESSAGE_H_
