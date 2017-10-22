#ifndef DSA_SDK_INCOMING_INVOKE_STREAM_H
#define DSA_SDK_INCOMING_INVOKE_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../message_io_stream.h"

namespace dsa {

class InvokeRequestMessage;
class InvokeResponseMessage;

class IncomingInvokeStream : public MessageQueueStream {
 public:
  typedef std::function<void(IncomingInvokeStream&,
                             ref_<const InvokeResponseMessage>&&)>
      Callback;

 protected:
  Callback _callback;

 public:
  IncomingInvokeStream(ref_<Session>&& session, const Path& path, uint32_t rid,
                       Callback&& callback);

  void receive_message(MessageCRef&& msg) override;

  void invoke(ref_<const InvokeRequestMessage>&& msg);

  void close();

  MessageCRef get_next_message(AckCallback& callback) override;
};
}

#endif  // DSA_SDK_INCOMING_INVOKE_STREAM_H
