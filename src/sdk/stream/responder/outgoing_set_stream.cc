#include "dsa_common.h"

#include "outgoing_set_stream.h"

#include "message/request/set_request_message.h"
namespace dsa {

OutgoingSetStream::OutgoingSetStream(
  ref_<Session> &&session, const Path &path, uint32_t rid,
  ref_<const SetRequestMessage> &&message)
  : MessageCacheStream(std::move(session), path, rid),_waiting_request(std::move(message)) {
}

void OutgoingSetStream::close_impl() {
  if (_callback != nullptr) {
    std::move(_callback)(*this, ref_<const SetRequestMessage>());
  }
}

void OutgoingSetStream::receive_message(MessageCRef &&mesage) {
  if (_callback != nullptr) {
    _callback(*this, std::move(mesage));
  } else {
    _waiting_request = std::move(mesage);
  }
};

void OutgoingSetStream::on_request(Callback &&callback) {
  _callback = std::move(callback);
  if (_callback != nullptr && _waiting_request != nullptr) {
      _callback(*this, std::move(_waiting_request));
  }
}
}