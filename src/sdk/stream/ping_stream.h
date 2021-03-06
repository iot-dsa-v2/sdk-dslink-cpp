#ifndef DSA_SDK_PING_STREAM_H
#define DSA_SDK_PING_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "message_io_stream.h"

namespace dsa {

class PingStream final : public MessageRefedStream {
  ref_<Message> _message;

 public:
  explicit PingStream(ref_<Session>&& session);
  size_t peek_next_message_size(size_t available, int64_t time) final;
  MessageCRef get_next_message(AckCallback& callbping) final;

  void receive_message(ref_<Message>&& msg) final {}

  void add_ping();

  bool disconnected() final { return true; }
};
}

#endif  // DSA_SDK_PING_STREAM_H
