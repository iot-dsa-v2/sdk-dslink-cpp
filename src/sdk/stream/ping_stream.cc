#include "dsa_common.h"

#include "ping_stream.h"

#include "core/session.h"

namespace dsa {
PingStream::PingStream(ref_<Session>&& session)
    : MessageRefedStream(std::move(session), Path()),
      _message(new Message(MessageType::PING)) {}

size_t PingStream::peek_next_message_size(size_t available, int64_t time) {
  return _message->size();
}
MessageCRef PingStream::get_next_message(AckCallback& callback) {
  _writing = false;
  return _message;
}

void PingStream::add_ping() {
  if (!_writing) {
    _writing = true;
    _session->write_stream(get_ref());
  }
}
}
