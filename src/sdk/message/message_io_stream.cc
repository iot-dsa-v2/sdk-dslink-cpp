#include "dsa_common.h"

#include "message_io_stream.h"

#include "core/session.h"

namespace dsa {

MessageIoStream::MessageIoStream(intrusive_ptr_<Session> &&session, uint32_t rid)
    : _session(std::move(session)), _rid(rid) {

}
MessageIoStream::~MessageIoStream() {

}

size_t MessageIoStream::peek_next_message_size() {
  if (_closed || _queue.empty()) {
    return 0;
  }
  return _queue.front()->size();
}
MessagePtr MessageIoStream::get_next_message() {
  if (_closed || _queue.empty()) {
    return nullptr;
  }
  MessagePtr msg = std::move(_queue.front());
  _queue.pop_front();

  if (!_queue.empty()) {
    _session->add_ready_stream(intrusive_this());
  }
  return msg;
}
}
