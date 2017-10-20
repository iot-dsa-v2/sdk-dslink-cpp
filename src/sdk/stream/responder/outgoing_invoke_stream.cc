#include "dsa_common.h"

#include "outgoing_invoke_stream.h"

#include "message/request/invoke_request_message.h"
namespace dsa {

OutgoingInvokeStream::OutgoingInvokeStream(
    ref_<Session> &&session, const Path &path, uint32_t rid,
    ref_<const InvokeRequestMessage> &&mesage)
    : MessageQueueStream(std::move(session), path, rid) {
  _waiting_requests.emplace_back(std::move(mesage));
}

void OutgoingInvokeStream::destroy_impl() {
  if (_callback != nullptr) {
    std::move(_callback)(*this, ref_<const InvokeRequestMessage>());
  }
}

void OutgoingInvokeStream::receive_message(MessageCRef &&mesage) {
  if (_callback != nullptr) {
    _callback(*this, std::move(mesage));
  } else {
    _waiting_requests.emplace_back(std::move(mesage));
  }
};

void OutgoingInvokeStream::on_request(Callback &&callback) {
  _callback = std::move(callback);
  if (_callback != nullptr && !_waiting_requests.empty()) {
    for (auto & msg : _waiting_requests) {
      _callback(*this, std::move(msg));
    }
    _waiting_requests.clear();
  }
}
}