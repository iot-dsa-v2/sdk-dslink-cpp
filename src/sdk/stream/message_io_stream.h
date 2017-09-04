#ifndef DSA_SDK_MESSAGE_IO_STREAM_H
#define DSA_SDK_MESSAGE_IO_STREAM_H

#include "message/base_message.h"

#include <deque>

namespace dsa {

class Session;

class MessageRefedStream : public MessageStream {
 protected:
  ref_<Session> _session;
  bool _writing = false;

 public:

  const std::string path;

  explicit MessageRefedStream(ref_<Session> &&session, const std::string &path,
                              uint32_t rid = 0);

  MessageRefedStream(const MessageRefedStream &other) = delete;
  MessageRefedStream(MessageRefedStream &&other) noexcept= delete;
  MessageRefedStream &operator=(const MessageRefedStream &other) = delete;
  MessageRefedStream &operator=(MessageRefedStream &&other) noexcept = delete;
  ~MessageRefedStream() override;
};

/// message stream with one message cache to write
class MessageCacheStream : public MessageRefedStream {
 protected:
  MessageCRef _cache;

  void close_impl() override;

 public:
  MessageCacheStream(ref_<Session> &&session, const std::string &path,
                     uint32_t rid = 0);
  ~MessageCacheStream() override;

  /// put message to writing cache
  void send_message(MessageCRef &&msg);

  size_t peek_next_message_size(size_t available) override;
  MessageCRef get_next_message(int32_t ack_id) override;
};

/// message stream with a queue to write
class MessageQueueStream : public MessageRefedStream {
 protected:
  std::deque<MessageCRef> _queue;

  void close_impl() override;

 public:
  explicit MessageQueueStream(ref_<Session> &&session, const std::string &path,
                              uint32_t rid = 0);
  ~MessageQueueStream() override;

  /// add message to the queue
  void send_message(MessageCRef &&msg);

  size_t peek_next_message_size(size_t available) override;
  MessageCRef get_next_message(int32_t ack_id) override;
};
}

#endif  // DSA_SDK_MESSAGE_IO_STREAM_H
