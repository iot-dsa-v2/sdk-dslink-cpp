#ifndef DSA_SDK_OUTGOING_LIST_STREAM_H
#define DSA_SDK_OUTGOING_LIST_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../message_io_stream.h"

#include "message/message_options.h"

namespace dsa {
class OutgoingListStream : public MessageRefedStream {
 public:
  typedef std::function<void(OutgoingListStream &)> CancelCallback;

 protected:
  CancelCallback _cancel_callback;

  void close_impl() override;

 public:
  OutgoingListStream(ref_<Session> &&session, const Path &path, uint32_t rid,
                     ListOptions &&options);


  void receive_message(MessageCRef &&mesage) override;
};
}

#endif  // DSA_SDK_OUTGOING_LIST_STREAM_H
