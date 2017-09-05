#include "dsa_common.h"

#include "requester.h"

#include "core/session.h"

#include "message/request/subscribe_request_message.h"

namespace dsa {

Requester::Requester(Session &session) : _session(session) {}

uint32_t Requester::next_rid() {
  while (_incoming_streams.find(++_next_rid) != _incoming_streams.end()) {
    // find next available get_rid;
  }
  if (_next_rid == 0) {
    // rid can't be 0, do it again;
    return next_rid();
  }
  return _next_rid;
}

void Requester::receive_message(MessageRef &&message) {
  auto search = _incoming_streams.find(message->get_rid());
  if (search != _incoming_streams.end()) {
    auto &stream = search->second;
    stream->receive_message(std::move(message));
  }
}
ref_<IncomingSubscribeStream> Requester::subscribe(
    const std::string &path, IncomingSubscribeStream::Callback &&callback,
    const SubscribeOptions &options) {
  uint32_t rid = next_rid();
  auto stream = make_ref_<IncomingSubscribeStream>(_session.get_ref(), Path(path),
                                                   rid, std::move(callback));
  _incoming_streams[rid] = stream;

  auto msg = make_ref_<SubscribeRequestMessage>();
  msg->set_subscribe_option(options);
  msg->set_target_path(path);
  stream->send_message(std::move(msg));

  return stream;
}
}
