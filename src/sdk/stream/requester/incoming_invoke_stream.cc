#include "dsa_common.h"

#include "incoming_invoke_stream.h"

#include "message/response/invoke_response_message.h"
#include "message/request/invoke_request_message.h"

namespace dsa {

IncomingInvokeStream::IncomingInvokeStream(ref_<Session>&& session,
                                           const Path& path, uint32_t rid,
                                           Callback&& callback)
    : MessageQueueStream(std::move(session), path, rid) {}

void IncomingInvokeStream::receive_message(MessageCRef&& msg) {
  if (msg->type() == MessageType::INVOKE_RESPONSE) {
    if (_callback != nullptr) {
      _callback(*this, std::move(msg));
    }
  }
}

void IncomingInvokeStream::invoke(ref_<const InvokeRequestMessage>&& msg) {
  send_message(MessageCRef(std::move(msg)));
}
}
