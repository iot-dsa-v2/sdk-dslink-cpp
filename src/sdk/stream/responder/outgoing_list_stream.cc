#include "dsa_common.h"

#include "outgoing_list_stream.h"

#include "core/session.h"

namespace dsa {
OutgoingListStream::OutgoingListStream(ref_<Session> &&session,
                                       const Path &path, uint32_t rid,
                                       ListOptions &&options)
    : MessageRefedStream(std::move(session), path, rid) {}
}
