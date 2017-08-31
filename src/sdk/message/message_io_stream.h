#ifndef DSA_SDK_MESSAGE_IO_STREAM_H
#define DSA_SDK_MESSAGE_IO_STREAM_H

#include "base_message.h"

#include <deque>

namespace dsa {

class Session;

class MessageRefedStream : public MessageStream {
 protected:
  ref_<Session> _session;
  uint32_t _rid;

  explicit MessageRefedStream(ref_<Session> &&session, uint32_t rid = 0);

 public:
  uint32_t get_rid() { return _rid; };
};

/// message stream with one message cache to write
class MessageCacheStream : public MessageRefedStream {
protected:
  MessageRef _cache;

  void close_impl() override;

public:
  MessageCacheStream(ref_<Session> &&session, uint32_t rid = 0);
  ~MessageCacheStream() override;

  size_t peek_next_message_size(size_t available) override;
  MessageRef get_next_message() override;
};

/// message stream with a queue to write
class MessageQueueStream : public MessageRefedStream {
 protected:
  std::deque<MessageRef> _queue;

  void close_impl() override;

 public:
  explicit MessageQueueStream(ref_<Session> &&session, uint32_t rid = 0);
  ~MessageQueueStream() override;

  size_t peek_next_message_size(size_t available) override;
  MessageRef get_next_message() override;
};
}

#endif  // DSA_SDK_MESSAGE_IO_STREAM_H
