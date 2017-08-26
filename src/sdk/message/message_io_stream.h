#ifndef DSA_SDK_MESSAGE_IO_STREAM_H
#define DSA_SDK_MESSAGE_IO_STREAM_H

#include "base_message.h"

#include <deque>

namespace dsa {

class Session;

class MessageIoStream : public MessageStream {
 protected:
  intrusive_ptr_<Session> _session;
  std::deque<MessagePtr> _queue;

  uint32_t _rid;

  std::function<void()> _on_close;

  void close_impl() override;

 public:
  MessageIoStream(intrusive_ptr_<Session> &&session, uint32_t rid = 0);
  ~MessageIoStream() override;

  uint32_t get_rid() { return _rid; };

  size_t peek_next_message_size(size_t available) override;
  MessagePtr get_next_message() override;


};
}

#endif  // DSA_SDK_MESSAGE_IO_STREAM_H
