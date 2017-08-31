#ifndef DSA_SDK_REQUESTER_H_
#define DSA_SDK_REQUESTER_H_

#include <map>

#include "incoming_message_stream.h"

namespace dsa {
class Session;
class SubscribeRequestMessage;

class Requester {
  friend class Session;

 protected:
  Session &_session;
  std::map<uint32_t, intrusive_ptr_<IncomingMessageStream>> _incoming_streams;

  void receive_message(MessagePtr &&message);

 public:
  Requester(Session &session);

  void new_subscribe(intrusive_ptr_<SubscribeRequestMessage> &&message);
};

}  // namespace dsa

#endif  // DSA_SDK_REQUESTER_H_
