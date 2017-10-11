#ifndef DSA_SDK_INCOMING_SUBSCRIBE_STREAM_H
#define DSA_SDK_INCOMING_SUBSCRIBE_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../message_io_stream.h"

#include "message/message_options.h"

namespace dsa {

class SubscribeResponseMessage;

class IncomingSubscribeStream : public MessageCacheStream {
 public:
  typedef std::function<void(ref_<const SubscribeResponseMessage>&&,
                             IncomingSubscribeStream&)>
      Callback;

  static const SubscribeOptions default_options;

 protected:
  Callback _callback;

 public:
  explicit IncomingSubscribeStream(ref_<Session>&& session,
                                   const Path& path, uint32_t rid,
                                   Callback&& callback);

  void receive_message(MessageCRef&& msg) override;

  // send subscription request and update of subscription options
  void subscribe(const SubscribeOptions &options);
};
}

#endif  // DSA_SDK_INCOMING_SUBSCRIBE_STREAM_H
