#ifndef DSA_SUBSCRIBE_REQUEST_MESSAGE_H_
#define DSA_SUBSCRIBE_REQUEST_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"
#include "message/message_options.h"

namespace dsa {

class SubscribeRequestMessage : public RequestMessage {
 protected:
  // measure the size and header size
  void update_static_header() override;
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const override;
  void parse_dynamic_data(const uint8_t* data, size_t dynamic_header_size,
                          size_t body_size) throw(const MessageParsingError&);

  std::unique_ptr<DynamicByteHeader> qos;
  // std::unique_ptr<DynamicByteHeader> update_frequency;
  std::unique_ptr<DynamicIntHeader> queue_size;
  std::unique_ptr<DynamicIntHeader> queue_time;

 public:
  explicit SubscribeRequestMessage(const uint8_t* data, size_t size);
  SubscribeRequestMessage();
  SubscribeRequestMessage(const SubscribeRequestMessage&);

  QosLevel get_qos() const;
  void set_qos(QosLevel value);

  int32_t get_queue_size() const;
  void set_queue_size(int32_t value);

  int32_t get_queue_time() const;
  void set_queue_time(int32_t value);

  SubscribeOptions get_subscribe_options() const;
  void set_subscribe_option(const SubscribeOptions& option);
};

}  // namespace dsa

#endif  // DSA_SUBSCRIBE_REQUEST_MESSAGE_H_
