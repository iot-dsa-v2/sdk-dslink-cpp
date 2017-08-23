#include "dsa_common.h"

#include "message_stream.h"

#include "session.h"

namespace dsa {

MessageStream::MessageStream(intrusive_ptr_<Session> &&session,
                             uint32_t request_id,
                             size_t unique_id)
    : _strand(session->_strand),
      _request_id(request_id),
      _unique_id(unique_id),
      _set_ready([ &, session = std::move(session) ]() {
        session->add_ready_stream(intrusive_this<MessageStream>());
      }) {}

void MessageStream::close() {
//  for (auto &holder : _holders) {
//    if (holder != nullptr)
//      holder->remove_stream(this);
//  }
  Closable::close();
}


}
