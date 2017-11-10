#ifndef DSA_SDK_PING_MESSAGE_H
#define DSA_SDK_PING_MESSAGE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "base_message.h"

namespace dsa {
class PingMessage final : public Message {
 public:
  PingMessage(const uint8_t* data, size_t size);
  PingMessage();

 protected:
  void update_static_header() final;
};
}

#endif  // DSA_SDK_PING_MESSAGE_H
