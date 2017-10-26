#ifndef DSA_SDK_INCOMING_INVOKE_STREAM_H
#define DSA_SDK_INCOMING_INVOKE_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../message_io_stream.h"

namespace dsa {

class InvokeRequestMessage;
class InvokeResponseMessage;

class IncomingInvokeStream final : public MessageQueueStream {
 public:
  typedef std::function<void(IncomingInvokeStream&,
                             ref_<const InvokeResponseMessage>&&)>
      Callback;

 protected:
  Callback _callback;

 public:
  IncomingInvokeStream(ref_<Session>&& session, const Path& path, uint32_t rid,
                       Callback&& callback);

  void receive_message(MessageCRef&& msg) final;

  void invoke(ref_<const InvokeRequestMessage>&& msg);

  void close();

  bool check_close_message(MessageCRef& message) final;
};
}

#endif  // DSA_SDK_INCOMING_INVOKE_STREAM_H
