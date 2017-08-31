#ifndef DSA_SDK_REQUESTER_H_
#define DSA_SDK_REQUESTER_H_

#include <map>

#include "incoming_message_stream.h"

#include "stream/incoming_subscribe_stream.h"

namespace dsa {

class Session;

class Requester {
  friend class Session;

 protected:
  Session &_session;
  std::map<uint32_t, ref_<IncomingMessageStream>> _incoming_streams;

  void receive_message(MessageRef &&message);

 public:
  Requester(Session &session);

  ref_<IncomingSubscribeStream> subscribe(
      ref_<SubscribeRequestMessage> &&message);
};

}  // namespace dsa

#endif  // DSA_SDK_REQUESTER_H_
