#include "dsa_common.h"

#include "outgoing_list_stream.h"

#include "core/session.h"
#include "module/logger.h"

namespace dsa {
OutgoingListStream::OutgoingListStream(ref_<Session> &&session,
                                       const Path &path, uint32_t rid,
                                       ListOptions &&options)
    : MessageRefedStream(std::move(session), path, rid) {}

void OutgoingListStream::close_impl() {
  if (_cancel_callback != nullptr) {
    std::move(_cancel_callback)(*this);
  };
}

void OutgoingListStream::receive_message(MessageCRef &&mesage) {
  LOG_ERROR(_session->get_strand()->logger(),
            LOG << "unexpected request update on list stream");
}
}
