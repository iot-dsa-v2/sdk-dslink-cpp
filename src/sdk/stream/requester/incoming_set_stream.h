#ifndef DSA_SDK_INCOMING_SET_STREAM_H
#define DSA_SDK_INCOMING_SET_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../message_io_stream.h"

namespace dsa {

class SetRequestMessage;
class SetResponseMessage;

class IncomingSetStream : public MessageCacheStream {
 public:
  typedef std::function<void(IncomingSetStream&,
                             ref_<const SetResponseMessage>&&)>
      Callback;

 protected:
  Callback _callback;

 public:
  IncomingSetStream(ref_<Session>&& session, const Path& path, uint32_t rid,
                    Callback&& callback);

  void receive_message(MessageCRef&& msg) override;

  void set(ref_<const SetRequestMessage>&& msg);
};
}

#endif  // DSA_SDK_INCOMING_SET_STREAM_H
