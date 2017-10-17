#ifndef DSA_SDK_OUTGOING_INVOKE_STREAM_H
#define DSA_SDK_OUTGOING_INVOKE_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <map>

#include "../message_io_stream.h"

#include "message/response/invoke_response_message.h"

namespace dsa {
class InvokeRequestMessage;

class OutgoingInvokeStream : public MessageQueueStream {
 public:
  typedef std::function<void(OutgoingInvokeStream &,
                             ref_<const InvokeRequestMessage> &&)>
      Callback;

 protected:
  Callback _callback;

  void close_impl() override;

 public:
  OutgoingInvokeStream(ref_<Session> &&session, const Path &path, uint32_t rid,
                       ref_<const InvokeRequestMessage> &&msg);

  void receive_message(MessageCRef &&mesage) override;

  void send_response(InvokeResponseMessageCRef &&message) {
    send_message(MessageCRef(std::move(message)));
  };
};
}

#endif  // DSA_SDK_OUTGOING_INVOKE_STREAM_H
