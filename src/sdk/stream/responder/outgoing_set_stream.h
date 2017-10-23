#ifndef DSA_SDK_OUTGOING_SET_STREAM_H
#define DSA_SDK_OUTGOING_SET_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <vector>

#include "../message_io_stream.h"

#include "message/response/set_response_message.h"

namespace dsa {
class SetRequestMessage;

class OutgoingSetStream : public MessageCacheStream {
 public:
  typedef std::function<void(OutgoingSetStream &,
                             ref_<const SetRequestMessage> &&)>
      Callback;

 protected:
  Callback _callback;

  ref_<const SetRequestMessage> _waiting_request;

  void destroy_impl() override;

 public:
  OutgoingSetStream(ref_<Session> &&session, const Path &path, uint32_t rid,
                    ref_<const SetRequestMessage> &&message);

  void receive_message(MessageCRef &&mesage) override;

  void on_request(Callback &&callback);

  void send_response(ref_<SetResponseMessage> &&message);

  bool check_close_message(MessageCRef &message) override;
};
}

#endif  // DSA_SDK_OUTGOING_SET_STREAM_H
