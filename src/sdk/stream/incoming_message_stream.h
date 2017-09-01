#ifndef DSA_SDK_SUBSCRIPTION_STREAM_H_
#define DSA_SDK_SUBSCRIPTION_STREAM_H_

#include "message_io_stream.h"

namespace dsa {

class IncomingMessageStream : public MessageRefedStream {
 public:
  IncomingMessageStream(ref_<Session> &&session, const std::string &path, uint32_t rid = 0);
  virtual ~IncomingMessageStream() = default;
};

}  // namespace dsa

#endif  // DSA_SDK_SUBSCRIPTION_STREAM_H_
