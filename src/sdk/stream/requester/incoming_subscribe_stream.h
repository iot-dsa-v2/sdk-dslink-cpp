#ifndef DSA_SDK_INCOMING_SUBSCRIBE_STREAM_H
#define DSA_SDK_INCOMING_SUBSCRIBE_STREAM_H

#include "../incoming_message_stream.h"

#include "message/message_options.h"

namespace dsa {

class SubscribeResponseMessage;

class IncomingSubscribeStream : public MessageCacheStream {
 public:
  typedef std::function<void(ref_<SubscribeResponseMessage>&&)> Callback;

  static const SubscribeOptions default_options;

 protected:
  Callback _callback;

 public:
  explicit IncomingSubscribeStream(ref_<Session>&& session,
                                   const std::string& path, Callback&& callback,
                                   uint32_t rid);
  void receive_message(MessageRef&& msg) override;
};
}

#endif  // DSA_SDK_INCOMING_SUBSCRIBE_STREAM_H
