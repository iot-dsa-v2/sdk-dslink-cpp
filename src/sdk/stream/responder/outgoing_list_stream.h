#ifndef DSA_SDK_OUTGOING_LIST_STREAM_H
#define DSA_SDK_OUTGOING_LIST_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>

#include "../message_io_stream.h"

#include "message/message_options.h"

namespace dsa {
class Var;

class OutgoingListStream : public MessageRefedStream {
 public:
  typedef std::function<void(OutgoingListStream &)> CancelCallback;

 protected:
  CancelCallback _cancel_callback;

  void destroy_impl() override;

  std::unordered_map<std::string, BytesRef> _cached_map;
  size_t _next_size;

 public:
  OutgoingListStream(ref_<Session> &&session, const Path &path, uint32_t rid,
                     ListOptions &&options);

  void update_value(const std::string &key, BytesRef &value);
  void update_value(const std::string &key, const Var &v);

  size_t peek_next_message_size(size_t available, int64_t time) override;
  MessageCRef get_next_message(AckCallback &) override;

  void receive_message(MessageCRef &&mesage) override;
};
}

#endif  // DSA_SDK_OUTGOING_LIST_STREAM_H
