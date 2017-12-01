#ifndef DSA_SDK_OUTGOING_SUBSCRIBE_STREAM_H
#define DSA_SDK_OUTGOING_SUBSCRIBE_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../message_io_stream.h"

#include "message/request/subscribe_request_message.h"
#include "message/response/subscribe_response_message.h"

namespace dsa {

class OutgoingSubscribeStream final : public MessageQueueStream {
 protected:
  SubOptionChangeCallback _option_callback;
  SubscribeOptions _options;
  void set_options(SubscribeOptions &&options);

  void check_queue_time(int64_t time) final;
  void check_queue_size() final;

  void destroy_impl() final;

 public:
  const SubscribeOptions &subscribe_options() final { return _options; }

  OutgoingSubscribeStream(ref_<Session> &&session, const Path &path,
                          uint32_t rid, SubscribeOptions &&options);

  void on_subscribe_option_change(SubOptionChangeCallback &&callback) final {
    _option_callback = callback;
  }

  void receive_message(ref_<Message> &&mesage) final;

  void send_subscribe_response(SubscribeResponseMessageCRef &&message) final {
    send_message(MessageCRef(std::move(message)));
  }
};
}

#endif  // DSA_SDK_OUTGOING_SUBSCRIBE_STREAM_H
