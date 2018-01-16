#include "dsa_common.h"

#include "incoming_invoke_stream.h"

#include "core/session.h"
#include "message/request/invoke_request_message.h"
#include "message/response/invoke_response_message.h"

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
      if (DOWN_CAST<const InvokeResponseMessage*>(message.get())->get_status() >=
          MessageStatus::CLOSED) {
        _closed = true;
      }
      _callback(*this, std::move(message));
    }
  }
}

void IncomingInvokeStream::invoke(ref_<const InvokeRequestMessage>&& msg) {
  send_message(MessageCRef(std::move(msg)));
}

void IncomingInvokeStream::close() {
  if (_closed) return;
  _closed = true;
  _callback = nullptr;
  user_callback = nullptr;
  send_message(make_ref_<RequestMessage>(MessageType::CLOSE_REQUEST), true);
}

bool IncomingInvokeStream::check_close_message(MessageCRef& message) {
  if (message->type() == MessageType::CLOSE_REQUEST) {
    _session->requester.remove_stream(rid);
    return true;
  }
  return false;
}

bool IncomingInvokeStream::disconnected() {
  if (_callback != nullptr) {
    auto response = make_ref_<InvokeResponseMessage>();
    response->set_status(MessageStatus::DISCONNECTED);
    _callback(*this, std::move(response));
  }
  destroy();
  return true;
}
}
