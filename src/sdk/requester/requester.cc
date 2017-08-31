#include "dsa_common.h"

#include "requester.h"

namespace dsa {
void Requester::receive_message(MessagePtr &&message) {
  auto search = _incoming_streams.find(message->request_id());
  if (search != _incoming_streams.end()) {
    auto &stream = search->second;
    stream->receive_message(std::move(message));
  }
}
void Requester::new_subscribe(intrusive_ptr_<SubscribeRequestMessage> message) {


}
}
