#ifndef DSA_SDK_OUTGOING_SUBSCRIBE_STREAM_H
#define DSA_SDK_OUTGOING_SUBSCRIBE_STREAM_H

#include "../message_io_stream.h"

#include "message/request/subscribe_request_message.h"
#include "message/response/subscribe_response_message.h"

namespace dsa {

class OutgoingSubscribeStream : public MessageQueueStream {
 public:
  typedef std::function<void(OutgoingSubscribeStream &)> Callback;

 protected:
  Callback _callback;
  SubscribeOptions _options;

  void close_impl() override;

 public:
  const SubscribeOptions &options() { return _options; }

  OutgoingSubscribeStream(ref_<Session> &&session, const std::string &path,
                          uint32_t rid, SubscribeOptions &&options);

  void on_update(Callback &&callback);

  void receive_message(MessageCRef &&mesage) override;

  void send_value(Variant &&value);
  void send_value(MessageValue &&value);
};
}

#endif  // DSA_SDK_OUTGOING_SUBSCRIBE_STREAM_H
