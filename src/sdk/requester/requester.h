#ifndef DSA_SDK_REQUESTER_H_
#define DSA_SDK_REQUESTER_H_

#include <map>

#include "incoming_message_stream.h"

namespace dsa {
class Session;

class Requester {
  friend class Session;

 protected:
  Session &_session;
  std::map<uint32_t, intrusive_ptr_<IncomingMessageStream>> _outgoing_streams;

 public:
  Requester(Session &session) : _session(session){};
};

}  // namespace dsa

#endif  // DSA_SDK_REQUESTER_H_
