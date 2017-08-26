#ifndef DSA_SDK_SUBSCRIPTION_STREAM_H_
#define DSA_SDK_SUBSCRIPTION_STREAM_H_

#include "message/message_io_stream.h"

namespace dsa {

class IncomingMessageStream : public MessageStream {
 public:
  virtual ~IncomingMessageStream() = default;
};

}  // namespace dsa

#endif  // DSA_SDK_SUBSCRIPTION_STREAM_H_
