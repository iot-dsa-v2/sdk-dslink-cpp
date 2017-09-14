#ifndef DSA_SDK_ACK_MESSAGE_H
#define DSA_SDK_ACK_MESSAGE_H

#include "base_message.h"

namespace dsa {
class AckMessage : public Message {
 public:
  AckMessage(const uint8_t* data, size_t size);
  AckMessage();

  void set_ack(int32_t ack) { _ack = ack; };

 protected:
  int32_t _ack;

  void update_static_header() override;
  void write_dynamic_data(uint8_t* data) const override;
};
}

#endif  // DSA_SDK_ACK_MESSAGE_H
