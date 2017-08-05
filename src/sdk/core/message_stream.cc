#include "dsa_common.h"

#include "message_stream.h"

#include "session.h"

namespace dsa {

MessageStream::MessageStream(const intrusive_ptr_<Session> &session, uint32_t request_id, size_t unique_id)
  : _strand(session->strand()), _request_id(request_id), _unique_id(unique_id) {
}

}
