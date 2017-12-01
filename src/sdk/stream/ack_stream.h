#ifndef DSA_SDK_ACK_STREAM_H
#define DSA_SDK_ACK_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "message_io_stream.h"

#include "message/ack_message.h"

namespace dsa {

class AckStream final : public MessageRefedStream {
  ref_<AckMessage> _message;

 public:
  explicit AckStream(ref_<Session>&& session);
  size_t peek_next_message_size(size_t available, int64_t time) final;
  MessageCRef get_next_message(AckCallback& callback) final;

  void receive_message(ref_<Message>&& msg) final {}

  void add_ack(int32_t ack);
  int32_t get_ack() const { return _message->get_ack(); };
};
}

#endif  // DSA_SDK_ACK_STREAM_H
