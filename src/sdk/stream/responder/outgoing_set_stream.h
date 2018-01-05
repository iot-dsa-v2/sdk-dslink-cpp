#ifndef DSA_SDK_OUTGOING_SET_STREAM_H
#define DSA_SDK_OUTGOING_SET_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <vector>

#include "../message_io_stream.h"
#include "message/message_page_group.h"
#include "message/response/set_response_message.h"

namespace dsa {
class SetRequestMessage;

class OutgoingSetStream final : public MessageCacheStream {
 public:
  typedef std::function<void(OutgoingSetStream &, ref_<SetRequestMessage> &&)>
      Callback;

 protected:
  // paged message that's partially received
  ref_<IncomingPagesMerger> _waiting_pages;

  Callback _callback;

  ref_<SetRequestMessage> _waiting_request;

  void destroy_impl() final;

 public:
  OutgoingSetStream(ref_<Session> &&session, const Path &path, uint32_t rid,
                    ref_<SetRequestMessage> &&message);

  void receive_message(ref_<Message> &&mesage) final;

  void on_request(Callback &&callback);

  void send_response(ref_<const SetResponseMessage> &&message);

  bool check_close_message(MessageCRef &message) final;
};
}

#endif  // DSA_SDK_OUTGOING_SET_STREAM_H
