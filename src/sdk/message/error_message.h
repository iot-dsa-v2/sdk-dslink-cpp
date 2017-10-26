#ifndef DSA_SDK_ERROR_MESSAGE_H
#define DSA_SDK_ERROR_MESSAGE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "base_message.h"

namespace dsa {


class ErrorMessage final : public ResponseMessage {
 public:
  ErrorMessage(MessageType type, MessageStatus status);
  void write_dynamic_data(uint8_t* data) const final;
  void update_static_header() final;
};
}
#endif //DSA_SDK_ERROR_MESSAGE_H
