#include "dsa_common.h"

#include "requester.h"

#include "core/session.h"

namespace dsa {

Requester::Requester(Session &session) : _session(session) {}

void Requester::close_impl() { _incoming_streams.clear(); }

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
  auto stream = make_ref_<IncomingSubscribeStream>(
      _session.get_ref(), Path(path), rid, std::move(callback));
  _incoming_streams[rid] = stream;

  stream->subscribe(options);

  return stream;
}

ref_<IncomingListStream> Requester::list(
    const std::string &path, IncomingListStream::Callback &&callback,
    const ListOptions &options) {
  uint32_t rid = next_rid();
  auto stream = make_ref_<IncomingListStream>(_session.get_ref(), Path(path),
                                              rid, std::move(callback));
  _incoming_streams[rid] = stream;

  stream->list(options);

  return stream;
}

ref_<IncomingListStream> Requester::invoke(
    const std::string &path, IncomingInvokeStream::Callback &&callback,
    ref_<const InvokeRequestMessage> &&message) {
  uint32_t rid = next_rid();
  auto stream = make_ref_<IncomingInvokeStream>(_session.get_ref(), Path(path),
                                                rid, std::move(callback));
  _incoming_streams[rid] = stream;

  stream->invoke(std::move(message));

  return stream;
}
}
