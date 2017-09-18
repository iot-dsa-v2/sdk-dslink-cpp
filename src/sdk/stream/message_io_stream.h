#ifndef DSA_SDK_MESSAGE_IO_STREAM_H
#define DSA_SDK_MESSAGE_IO_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "message/base_message.h"

#include <deque>

namespace dsa {

class Session;

class MessageRefedStream : public MessageStream {
 protected:
  ref_<Session> _session;
  bool _writing = false;

 public:
  const Path path;

  explicit MessageRefedStream(ref_<Session> &&session, const Path &path,
                              uint32_t rid = 0);

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

  void close_impl() override;

  /// put message to writing cache
  void send_message(MessageCRef &&msg);

 public:
  MessageCacheStream(ref_<Session> &&session, const Path &path,
                     uint32_t rid = 0);
  ~MessageCacheStream() override;

  size_t peek_next_message_size(size_t available) override;
  MessageCRef get_next_message(AckCallback &callback) override;
};

/// message stream with a queue to write
class MessageQueueStream : public MessageRefedStream {
 protected:
  std::deque<MessageCRef> _queue;

  void close_impl() override;

  /// add message to the queue
  void send_message(MessageCRef &&msg);

 public:
  explicit MessageQueueStream(ref_<Session> &&session, const Path &path,
                              uint32_t rid = 0);
  ~MessageQueueStream() override;



  size_t peek_next_message_size(size_t available) override;
  MessageCRef get_next_message(AckCallback &callback) override;
};
}

#endif  // DSA_SDK_MESSAGE_IO_STREAM_H
