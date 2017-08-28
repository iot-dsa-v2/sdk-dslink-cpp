#include "dsa_common.h"

#include "message_io_stream.h"

#include "core/session.h"

namespace dsa {

MessageIoStream::MessageIoStream(intrusive_ptr_<Session> &&session, uint32_t rid)
    : _session(std::move(session)), _rid(rid) {

}
MessageIoStream::~MessageIoStream() {

}

void MessageIoStream::close_impl() {
 if (_on_close != nullptr) {
   _on_close();
 }
  _queue.clear();
  _session.reset();
}

size_t MessageIoStream::peek_next_message_size(size_t available) {
  if (is_closed() || _queue.empty()) {
    return 0;
  }
  return _queue.front()->size();
}
MessagePtr MessageIoStream::get_next_message() {
  if (is_closed() || _queue.empty()) {
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