#ifndef DSA_SDK_SET_RESPONSE_MESSAGE_H_
#define DSA_SDK_SET_RESPONSE_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {

class SetResponseMessage : public ResponseMessage {
 public:
  SetResponseMessage(const SharedBuffer& buffer);

 public:
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const;
  // measure the size and header size
  void update_static_header();

 protected:
  void parse_dynamic_headers(const uint8_t* data, size_t size);
};

}  // namespace dsa

#endif  // DSA_SDK_SET_RESPONSE_MESSAGE_H_
