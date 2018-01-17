#ifndef DSA_SDK_INCOMING_SET_STREAM_H
#define DSA_SDK_INCOMING_SET_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../message_io_stream.h"

namespace dsa {

class SetRequestMessage;
class SetResponseMessage;

class IncomingSetStream final : public MessageQueueStream {
 public:
  typedef std::function<void(IncomingSetStream&,
                             ref_<const SetResponseMessage>&&)>
      Callback;

 protected:
  Callback _callback;
  bool _callback_running = false;

 public:
  IncomingSetStream(ref_<Session>&& session, const Path& path, uint32_t rid,
                    Callback&& callback);

  void receive_message(ref_<Message>&& msg) final;

  void set(ref_<const SetRequestMessage>&& msg);

  void close();
  bool check_close_message(MessageCRef& message) final;

  bool disconnected() final;
};
}

#endif  // DSA_SDK_INCOMING_SET_STREAM_H
