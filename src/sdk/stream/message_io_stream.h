#ifndef DSA_SDK_MESSAGE_IO_STREAM_H
#define DSA_SDK_MESSAGE_IO_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <deque>
#include "message/base_message.h"

#define BEFORE_CALLBACK_RUN()                                                \
  if (DSA_DEBUG && _callback_running) {                                      \
    LOG_FATAL(__FILENAME__,                                                  \
              LOG << "recursive callback: " << __FILE__ << " " << __func__); \
  }                                                                          \
  _callback_running = true;

#define AFTER_CALLBACK_RUN()            \
  if (is_closed()) _callback = nullptr; \
  _callback_running = false;

namespace dsa {
class Session;
class BaseSession;
class OutgoingPages;

class MessageRefedStream : public MessageStream {
 protected:
  ref_<BaseSession> _session;
  bool _closed = false;

  void destroy_impl() override;

 public:
  const Path path;
  bool is_closed() const { return _closed; }

  explicit MessageRefedStream(ref_<BaseSession> &&session, const Path &path,
                              uint32_t rid = 0);

  void send_message();
  // queue the sending, but not send message at once
  void post_message();

  // when return true, message will be closed
  // in that case the function itself should remove the stream from the rid map
  virtual bool check_close_message(MessageCRef &message) { return false; };

  MessageRefedStream(const MessageRefedStream &other) = delete;
  MessageRefedStream(MessageRefedStream &&other) noexcept = delete;
  MessageRefedStream &operator=(const MessageRefedStream &other) = delete;
  MessageRefedStream &operator=(MessageRefedStream &&other) noexcept = delete;
  ~MessageRefedStream() override;

  // force the stream to be the 1st in write queue
  // the existing item in queue won't be removed
  void make_critical();
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
    DEFAULT_MAX_QUEUE_SIZE = 0x10000,  // 64K
    DEFAULT_MAX_QUEUE_TIME = 120
  };

 protected:
  std::deque<MessageCRef> _queue;
  ref_<OutgoingPages> _waiting_page_group;

  void destroy_impl() override;

  /// add message to the queue
  void send_message(MessageCRef &&msg, bool close = false);

  int32_t _max_queue_size = DEFAULT_MAX_QUEUE_SIZE;
  int32_t _current_queue_size = 0;

  int32_t _max_queue_duration = DEFAULT_MAX_QUEUE_TIME;
  int64_t _current_queue_time = 0;

  // clear all element but the last
  void purge();

  virtual void check_queue_time(int64_t time);
  virtual void check_queue_size();

 public:
  explicit MessageQueueStream(ref_<Session> &&session, const Path &path,
                              uint32_t rid = 0);
  ~MessageQueueStream() override;

  size_t peek_next_message_size(size_t available, int64_t time) override;
  MessageCRef get_next_message(AckCallback &callback) override;
};
}  // namespace dsa

#endif  // DSA_SDK_MESSAGE_IO_STREAM_H
