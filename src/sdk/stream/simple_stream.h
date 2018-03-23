#ifndef DSA_SDK_ERROR_STREAM_H
#define DSA_SDK_ERROR_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "message/base_message.h"

#include "message/error_message.h"

namespace dsa {

class SimpleStream final : public MessageStream {
  MessageRef _message;

 public:
  SimpleStream(int32_t rid, MessageType type, Status status);
  SimpleStream(int32_t rid, MessageRef&& msg);
  size_t peek_next_message_size(size_t available, int64_t time) final;
  MessageCRef get_next_message(AckCallback& callback) final;

  void receive_message(ref_<Message>&& msg) final {}

  // when remove is disconnected

};
}

#endif  // DSA_SDK_ERROR_STREAM_H
