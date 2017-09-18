#ifndef DSA_SDK_ERROR_MESSAGE_H
#define DSA_SDK_ERROR_MESSAGE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "base_message.h"

namespace dsa {


class ErrorMessage :public ResponseMessage {
 public:
  ErrorMessage(MessageType type, MessageStatus status);
  void write_dynamic_data(uint8_t* data) const override;
  void update_static_header() override;
};
}
#endif //DSA_SDK_ERROR_MESSAGE_H
