#include "dsa_common.h"

#include "ack_stream.h"

#include "core/session.h"

namespace dsa {
AckStream::AckStream(ref_<Session>&& session)
    : MessageRefedStream(std::move(session), Path()),
      _message(new AckMessage()) {}

size_t AckStream::peek_next_message_size(size_t available, int64_t time) {
  return _message->size();
}
MessageCRef AckStream::get_next_message(AckCallback& callback) {
  return _message;
}

void AckStream::add_ack(int32_t ack) {
  _message->set_ack(ack);
  if (!_writing) {
    _session->write_stream(get_ref());
  }
}
}