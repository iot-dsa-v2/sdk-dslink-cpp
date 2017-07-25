#ifndef DSA_SUBSCRIBE_INVOKE_REQUEST_MESSAGE_H_
#define DSA_SUBSCRIBE_INVOKE_REQUEST_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {

class SubscribeRequestMessage : public RequestMessage {
 public:
  SubscribeRequestMessage(const SharedBuffer& buffer);

  std::unique_ptr<DynamicByteHeader> qos;
  std::unique_ptr<DynamicByteHeader> update_frequency;
  std::unique_ptr<DynamicByteHeader> queue_size;
  std::unique_ptr<DynamicByteHeader> queue_time;

 public:
  // measure the size and header size
  void update_static_header();

 protected:
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const;
  void parse_dynamic_headers(const uint8_t* data, size_t size);
};

}  // namespace dsa

#endif  // DSA_SUBSCRIBE_INVOKE_REQUEST_MESSAGE_H_
