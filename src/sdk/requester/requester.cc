#include "dsa_common.h"

#include "requester.h"

#include "message/request/subscribe_request_message.h"

namespace dsa {

Requester::Requester(Session &session) : _session(session) {

}
void Requester::receive_message(MessageRef &&message) {
  auto search = _incoming_streams.find(message->request_id());
  if (search != _incoming_streams.end()) {
    auto &stream = search->second;
    stream->receive_message(std::move(message));
  }
}
ref_<IncomingSubscribeStream> Requester::subscribe(ref_<SubscribeRequestMessage> &&message) {

  return ref_<IncomingSubscribeStream>();
}
}
