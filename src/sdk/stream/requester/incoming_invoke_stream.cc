#include "dsa_common.h"

#include "incoming_invoke_stream.h"

#include "core/session.h"
#include "message/request/invoke_request_message.h"
#include "message/response/invoke_response_message.h"
#include "module/logger.h"

namespace dsa {

IncomingInvokeStream::IncomingInvokeStream(ref_<Session>&& session,
                                           const Path& path, uint32_t rid,
                                           Callback&& callback)
    : MessageQueueStream(std::move(session), path, rid),
      _callback(std::move(callback)) {}

void IncomingInvokeStream::receive_message(ref_<Message>&& message) {
  if (message->type() == MessageType::INVOKE_RESPONSE) {
    IncomingPagesMerger::check_merge(_waiting_pages, message);
    if (_callback != nullptr) {
      if (DOWN_CAST<const InvokeResponseMessage*>(message.get())
              ->get_status() >= Status::DONE) {
        _closed = true;
      }
      BEFORE_CALLBACK_RUN();
      _callback(*this, std::move(message));
      AFTER_CALLBACK_RUN();
    }
  }
}

void IncomingInvokeStream::invoke(ref_<const InvokeRequestMessage>&& msg) {
  send_message(MessageCRef(std::move(msg)));
}

void IncomingInvokeStream::close() {
  if (_closed) return;
  _closed = true;
  if (!_callback_running) {
    _callback = nullptr;
  }
  send_message(make_ref_<RequestMessage>(MessageType::CLOSE_REQUEST), true);
}

bool IncomingInvokeStream::check_close_message(MessageCRef& message) {
  if (message->type() == MessageType::CLOSE_REQUEST) {
    _session->destroy_req_stream(rid);
    return true;
  }
  return false;
}

bool IncomingInvokeStream::disconnected() {
  if (_callback != nullptr) {
    auto response = make_ref_<InvokeResponseMessage>();
    response->set_status(Status::DISCONNECTED);
    BEFORE_CALLBACK_RUN();
    _callback(*this, std::move(response));
    AFTER_CALLBACK_RUN();
  }
  destroy();
  return true;
}
}
