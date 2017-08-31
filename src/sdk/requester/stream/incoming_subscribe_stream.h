#ifndef DSA_SDK_INCOMING_SUBSCRIBE_STREAM_H
#define DSA_SDK_INCOMING_SUBSCRIBE_STREAM_H

#include "../incoming_message_stream.h"

namespace dsa {

class SubscribeResponseMessage;

class IncomingSubscribeStream : public MessageCacheStream {
public:
  typedef std::function<void(ref_<SubscribeResponseMessage> &&)> Callback;

protected:
  Callback _callback;
public:
  explicit IncomingSubscribeStream(ref_<Session> &&session, uint32_t rid = 0);
  void receive_message(MessageRef&& msg) override;
};
}

#endif  // DSA_SDK_INCOMING_SUBSCRIBE_STREAM_H
