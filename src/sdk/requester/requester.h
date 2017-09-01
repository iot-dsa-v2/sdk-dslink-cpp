#ifndef DSA_SDK_REQUESTER_H_
#define DSA_SDK_REQUESTER_H_

#include <map>

#include "stream/incoming_message_stream.h"

#include "stream/requester/incoming_subscribe_stream.h"

namespace dsa {

class Session;

class Requester {
  friend class Session;

 protected:
  Session &_session;
  uint32_t _next_rid = 0;
  uint32_t next_rid();

  std::map<uint32_t, ref_<MessageStream>> _incoming_streams;

  void receive_message(MessageRef &&message);

 public:
  Requester(Session &session);

  ref_<IncomingSubscribeStream> subscribe(
      const std::string& path, IncomingSubscribeStream::Callback &&callback,
      const SubscribeOptions &options =
          IncomingSubscribeStream::default_options);
};

}  // namespace dsa

#endif  // DSA_SDK_REQUESTER_H_
