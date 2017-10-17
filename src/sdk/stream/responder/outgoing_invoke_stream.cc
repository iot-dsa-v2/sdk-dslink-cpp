#include "dsa_common.h"

#include "outgoing_invoke_stream.h"

#include "message/request/invoke_request_message.h"
namespace dsa {

OutgoingInvokeStream::OutgoingInvokeStream(
    ref_<Session> &&session, const Path &path, uint32_t rid,
    ref_<const InvokeRequestMessage> &&msg)
    : MessageQueueStream(std::move(session), path, rid) {
  _callback(*this, std::move(msg));
}

void OutgoingInvokeStream::close_impl() {
  if (_callback != nullptr) {
    std::move(_callback)(*this, ref_<const InvokeRequestMessage>());
  };
}

void OutgoingInvokeStream::receive_message(MessageCRef &&mesage) {
  _callback(*this, std::move(mesage));
};
}