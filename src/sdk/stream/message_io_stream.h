#ifndef DSA_SDK_MESSAGE_IO_STREAM_H
#define DSA_SDK_MESSAGE_IO_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <deque>
#include "message/base_message.h"

namespace dsa {

class Session;

class MessageRefedStream : public MessageStream {
 protected:
  ref_<Session> _session;
  bool _writing = false;
  bool _closed = false;

  void destroy_impl() override;

 public:
  const Path path;
  bool is_closed() const { return _closed; }

  explicit MessageRefedStream(ref_<Session> &&session, const Path &path,
                              uint32_t rid = 0);

  void send_message();

  MessageRefedStream(const MessageRefedStream &other) = delete;
  MessageRefedStream(MessageRefedStream &&other) noexcept = delete;
  MessageRefedStream &operator=(const MessageRefedStream &other) = delete;
  MessageRefedStream &operator=(MessageRefedStream &&other) noexcept = delete;
  ~MessageRefedStream() override;
};

/// message stream with one message cache to write
class MessageCacheStream : public MessageRefedStream {
 protected:
  MessageCRef _cache;

  void destroy_impl() override;

  /// put message to writing cache
  void send_message(MessageCRef &&msg, bool close = false);

 public:
  MessageCacheStream(ref_<Session> &&session, const Path &path,
                     uint32_t rid = 0);
  ~MessageCacheStream() override;

  size_t peek_next_message_size(size_t available, int64_t time) override;
  MessageCRef get_next_message(AckCallback &callback) override;
};

/// message stream with a queue to write
class MessageQueueStream : public MessageRefedStream {
 public:
  enum : int32_t {
    DEFAULT_MAX_QUEUE_SIZE = 65536,
    DEFAULT_MAX_QUEUE_TIME = 120
  };

 protected:
  std::deque<MessageCRef> _queue;

  void destroy_impl() override;

  /// add message to the queue
  void send_message(MessageCRef &&msg, bool close = false);

  int32_t _max_queue_size = DEFAULT_MAX_QUEUE_SIZE;
  int32_t _current_queue_size = 0;

  int64_t _current_queue_time = DEFAULT_MAX_QUEUE_TIME;
  int32_t _max_queue_duration = 0;

  // clear all element but the last
  void purge();

  virtual void check_queue_time(int64_t time) { purge(); }
  virtual void check_queue_size() { purge(); }

 public:
  explicit MessageQueueStream(ref_<Session> &&session, const Path &path,
                              uint32_t rid = 0);
  ~MessageQueueStream() override;

  size_t peek_next_message_size(size_t available, int64_t time) override;
  MessageCRef get_next_message(AckCallback &callback) override;
};
}

#endif  // DSA_SDK_MESSAGE_IO_STREAM_H
