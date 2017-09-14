#ifndef DSA_SDK_ACK_STREAM_H
#define DSA_SDK_ACK_STREAM_H

#include "message_io_stream.h"

#include "message/ack_message.h"

namespace dsa {

class AckStream : public MessageRefedStream {
  AckMessage _message;

 public:
  explicit AckStream(ref_<Session>&& session);
  size_t peek_next_message_size(size_t available) override;
  MessageCRef get_next_message(int32_t ack_id) override;

  void receive_message(MessageCRef&& msg) override {}

  void add_ack(int32_t ack);
};
}

#endif  // DSA_SDK_ACK_STREAM_H
