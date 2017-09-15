#include "dsa_common.h"

#include "ack_stream.h"

#include "core/session.h"

namespace dsa {
AckStream::AckStream(ref_<Session>&& session)
    : MessageRefedStream(std::move(session), Path()) {
  // keep a ref so it won't be deleted by smart pointer
  intrusive_ptr_add_ref(&_message);
}

size_t AckStream::peek_next_message_size(size_t available) {
  return _message.size();
}
MessageCRef AckStream::get_next_message(AckCallback& callback) {
  _writing = false;
  return _message.get_ref();
}

void AckStream::add_ack(int32_t ack) {
  _message.set_ack(ack);
  if (!_writing) {
    _writing = true;
    _session->write_stream(get_ref());
  }
}
}