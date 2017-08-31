#include "dsa_common.h"

#include "requester.h"

#include "core/session.h"

#include "message/request/subscribe_request_message.h"

namespace dsa {

Requester::Requester(Session &session) : _session(session) {}

uint32_t Requester::next_rid() {
  while (_incoming_streams.find(++_next_rid) != _incoming_streams.end()) {
    // find next available rid;
  }
  if (_next_rid == 0) {
    // rid can't be 0, do it again;
    return next_rid();
  }
  return _next_rid;
}

void Requester::receive_message(MessageRef &&message) {
  auto search = _incoming_streams.find(message->request_id());
  if (search != _incoming_streams.end()) {
    auto &stream = search->second;
    stream->receive_message(std::move(message));
  }
}
ref_<IncomingSubscribeStream> Requester::subscribe() {
  uint32_t rid = next_rid();
  auto stream = make_ref_<IncomingSubscribeStream>(_session.get_ref(), rid);
  _incoming_streams[rid] = stream;
  return stream;
}
}
