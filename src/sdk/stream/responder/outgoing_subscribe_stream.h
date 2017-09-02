#ifndef DSA_SDK_OUTGOING_SUBSCRIBE_STREAM_H
#define DSA_SDK_OUTGOING_SUBSCRIBE_STREAM_H

#include "../outgoing_message_stream.h"

#include "message/response/subscribe_response_message.h"

namespace dsa {
class SubscribeRequestMessage;

class OutgoingSubscribeStream : public OutgoingMessageStream {
 public:
  typedef std::function<void(ref_<SubscribeRequestMessage> &&,
                             OutgoingSubscribeStream &stream)>
      Callback;

 public:
  const SubscribeOptions options;

  OutgoingSubscribeStream(ref_<Session> &&session, const std::string &path,
                          uint32_t rid, SubscribeOptions &&options);
  void new_message(const SubscribeResponseMessage &new_message);

  void receive_message(MessageRef &&mesage) override;
};
}

#endif  // DSA_SDK_OUTGOING_SUBSCRIBE_STREAM_H
