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
  void set_options(SubscribeOptions && options);

  int32_t _max_queue_size = 0;
  int32_t _current_queue_size = 0;

  int32_t _max_queue_time = 0;
  int32_t _current_queue_time = 0;

  void check_queue_time();
  void check_queue_size();

  void close_impl() override;

 public:
  const SubscribeOptions &options() { return _options; }

  OutgoingSubscribeStream(ref_<Session> &&session, const Path &path,
                          uint32_t rid, SubscribeOptions &&options);

  void on_option_change(Callback &&callback);

  void receive_message(MessageCRef &&mesage) override;

  void send_response(SubscribeResponseMessageCRef &&message);

  MessageCRef get_next_message(AckCallback &callback) override;
};
}

#endif  // DSA_SDK_OUTGOING_SUBSCRIBE_STREAM_H
