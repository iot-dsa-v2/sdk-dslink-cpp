#include "dsa_common.h"

#include "incoming_message_stream.h"

#include "core/session.h"

namespace dsa {
IncomingMessageStream::IncomingMessageStream(ref_<Session> &&session,
                                             uint32_t rid)
    : MessageRefedStream(std::move(session), rid) {}
}
