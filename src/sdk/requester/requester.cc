#include "dsa_common.h"

#include "core/session.h"

#include "message/request/invoke_request_message.h"
#include "message/request/set_request_message.h"
#include "stream/requester/incoming_invoke_stream.h"
#include "stream/requester/incoming_list_stream.h"
#include "stream/requester/incoming_set_stream.h"
#include "stream/requester/incoming_subscribe_stream.h"

namespace dsa {



void Session::requester_disconnected() {
  for (auto it = _incoming_streams.begin(); it != _incoming_streams.end();) {
    if (it->second->disconnected()) {
      it = _incoming_streams.erase(it);
    } else {
      it->second->_writing = false;
      ++it;
    }
  }
}

int32_t Session::next_rid() {
  while (_incoming_streams.find(++_next_rid) != _incoming_streams.end()) {
    // find next available get_rid;
  }
  if (_next_rid < 0) {
    _next_rid = 1;
  }
  return _next_rid;
}

void Session::receive_req_message(MessageRef &&message) {
  auto search = _incoming_streams.find(message->get_rid());
  if (search != _incoming_streams.end()) {
    auto &stream = search->second;

    if (DOWN_CAST<ResponseMessage *>(message.get())->get_status() >=
        Status::DONE) {
      stream->receive_message(std::move(message));
      stream->destroy();
      _incoming_streams.erase(search);
    } else {
      stream->receive_message(std::move(message));
    }
  }
}
ref_<IncomingSubscribeStream> Session::subscribe(
    const string_ &path, IncomingSubscribeStreamCallback &&callback,
    const SubscribeOptions &options) {
  int32_t rid = next_rid();
  auto stream = make_ref_<IncomingSubscribeStream>(
      get_ref(), Path(path), rid, std::move(callback));
  _incoming_streams[rid] = stream;

  stream->subscribe(options);

  return stream;
}

ref_<IncomingListStream> Session::list(const string_ &path,
                                         IncomingListStreamCallback &&callback,
                                         const ListOptions &options) {
  int32_t rid = next_rid();
  auto stream = make_ref_<IncomingListStream>(get_ref(), Path(path),
                                              rid, std::move(callback));
  _incoming_streams[rid] = stream;

  stream->list(options);

  return stream;
}

ref_<IncomingInvokeStream> Session::invoke(
    IncomingInvokeStreamCallback &&callback,
    ref_<const InvokeRequestMessage> &&message) {
  int32_t rid = next_rid();
  auto stream = make_ref_<IncomingInvokeStream>(
      get_ref(), Path(message->get_target_path()), rid,
      std::move(callback));
  _incoming_streams[rid] = stream;

  stream->invoke(std::move(message));

  return stream;
}

ref_<IncomingSetStream> Session::set(
    IncomingSetStreamCallback &&callback,
    ref_<const SetRequestMessage> &&message) {
  int32_t rid = next_rid();
  auto stream = make_ref_<IncomingSetStream>(get_ref(),
                                             Path(message->get_target_path()),
                                             rid, std::move(callback));
  _incoming_streams[rid] = stream;

  stream->set(std::move(message));

  return stream;
}

bool Session::destroy_req_stream(int32_t rid) {
  auto search = _incoming_streams.find(rid);
  if (search != _incoming_streams.end()) {
    auto &stream = search->second;
    stream->destroy();
    _incoming_streams.erase(search);
    return true;
  }
  return false;
}
}
