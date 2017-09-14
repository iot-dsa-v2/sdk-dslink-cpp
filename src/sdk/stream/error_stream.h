#ifndef DSA_SDK_ERROE_STREAM_H
#define DSA_SDK_ERROE_STREAM_H

#include "message/base_message.h"

#include "message/error_message.h"

namespace dsa {

class ErrorStream : public MessageStream {
  ErrorMessage message;
 public:
  explicit ErrorStream(int32_t rid, MessageType type, MessageStatus status);
  size_t peek_next_message_size(size_t available) override;
  MessageCRef get_next_message(int32_t ack_id) override;

  void receive_message(MessageCRef&& msg) override {}
};
}

#endif  // DSA_SDK_ERROE_STREAM_H
