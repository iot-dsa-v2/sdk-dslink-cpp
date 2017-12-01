#ifndef DSA_SDK_ERROR_STREAM_H
#define DSA_SDK_ERROR_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "message/base_message.h"

#include "message/error_message.h"

namespace dsa {

class ErrorStream final : public MessageStream {
  ref_<ErrorMessage> _message;

 public:
  explicit ErrorStream(int32_t rid, MessageType type, MessageStatus status);
  size_t peek_next_message_size(size_t available, int64_t time) final;
  MessageCRef get_next_message(AckCallback& callback) final;

  void receive_message(ref_<Message>&& msg) final {}
};
}

#endif  // DSA_SDK_ERROR_STREAM_H
