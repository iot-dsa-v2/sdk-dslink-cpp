#ifndef DSA_SDK_ACK_MESSAGE_H
#define DSA_SDK_ACK_MESSAGE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "base_message.h"

namespace dsa {
class AckMessage final : public Message {
 public:
  AckMessage(const uint8_t* data, size_t size);
  AckMessage();

  void set_ack(int32_t ack) { _ack = ack; };
  int32_t get_ack() const { return _ack; }

  void print_message(std::ostream& os, int32_t rid) const final;

 protected:
  int32_t _ack;

  void update_static_header() final;
  void write_dynamic_data(uint8_t* data) const final;
};
}

#endif  // DSA_SDK_ACK_MESSAGE_H
