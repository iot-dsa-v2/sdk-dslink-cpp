#ifndef DSA_SDK_OUTGOING_SUBSCRIBE_STREAM_H
#define DSA_SDK_OUTGOING_SUBSCRIBE_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../message_io_stream.h"

#include "message/request/subscribe_request_message.h"
#include "message/response/subscribe_response_message.h"

namespace dsa {

class OutgoingSubscribeStream : public MessageQueueStream {
 public:
  typedef std::function<void(OutgoingSubscribeStream &,
                             const SubscribeOptions &)>
      Callback;

 protected:
  Callback _option_callback;
  SubscribeOptions _options;
  void set_options(SubscribeOptions &&options);

  void check_queue_time(int64_t time) override;
  void check_queue_size() override;

  void destroy_impl() override;

 public:
  const SubscribeOptions &options() { return _options; }

  OutgoingSubscribeStream(ref_<Session> &&session, const Path &path,
                          uint32_t rid, SubscribeOptions &&options);

  void on_option_change(Callback &&callback);

  void receive_message(MessageCRef &&mesage) override;

  void send_response(SubscribeResponseMessageCRef &&message) {
    send_message(MessageCRef(std::move(message)));
  }
};
}

#endif  // DSA_SDK_OUTGOING_SUBSCRIBE_STREAM_H
