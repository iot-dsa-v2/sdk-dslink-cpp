#ifndef DSA_SUBSCRIBE_REQUEST_MESSAGE_H_
#define DSA_SUBSCRIBE_REQUEST_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"
#include "../subscribe_option.h"

namespace dsa {

class SubscribeRequestMessage : public RequestMessage {
 public:
  SubscribeRequestMessage(const SharedBuffer& buffer);
  SubscribeRequestMessage();
  SubscribeRequestMessage(const SubscribeRequestMessage&);

 public:
  // measure the size and header size
  void update_static_header();

 protected:
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const;
  void parse_dynamic_headers(const uint8_t* data, size_t size);

 protected:
  std::unique_ptr<DynamicByteHeader> qos;
  // std::unique_ptr<DynamicByteHeader> update_frequency;
  std::unique_ptr<DynamicIntHeader> queue_size;
  std::unique_ptr<DynamicIntHeader> queue_time;

 public:
  SubscribeOption::Qos get_qos() const;
  void set_qos(SubscribeOption::Qos value);

  int32_t get_queue_size() const;
  void set_queue_size(int32_t value);

  int32_t get_queue_time() const;
  void set_queue_time(int32_t value);

  SubscribeOption get_subscribe_option() const;
  void set_subscribe_option(const SubscribeOption& option);
};

}  // namespace dsa

#endif  // DSA_SUBSCRIBE_REQUEST_MESSAGE_H_
