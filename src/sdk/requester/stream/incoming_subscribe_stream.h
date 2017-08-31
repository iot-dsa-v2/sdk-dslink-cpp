#ifndef DSA_SDK_INCOMING_SUBSCRIBE_STREAM_H
#define DSA_SDK_INCOMING_SUBSCRIBE_STREAM_H

#include "../incoming_message_stream.h"

namespace dsa {

class SubscribeRequestMessage;

class IncomingSubscribeStream : public IncomingMessageStream {
public:
  typedef std::function<void(ref_<SubscribeRequestMessage>, bool)> Callback;
};
}

#endif  // DSA_SDK_INCOMING_SUBSCRIBE_STREAM_H
