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

class OutgoingListStream final : public MessageRefedStream {
 protected:
  ListCloseCallback _close_callback;

  void destroy_impl() final;

  std::unordered_map<string_, BytesRef> _cached_map;
  size_t _next_size;

 public:
  OutgoingListStream(ref_<Session> &&session, const Path &path, uint32_t rid,
                     ListOptions &&options);

  void on_list_close(ListCloseCallback &&callback) final;

  void update_list_value(const string_ &key, BytesRef &value) final;

  size_t peek_next_message_size(size_t available, int64_t time) final;
  MessageCRef get_next_message(AckCallback &) final;

  void receive_message(MessageCRef &&mesage) final;
};
}

#endif  // DSA_SDK_OUTGOING_LIST_STREAM_H
