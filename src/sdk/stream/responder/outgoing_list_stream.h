#ifndef DSA_SDK_OUTGOING_LIST_STREAM_H
#define DSA_SDK_OUTGOING_LIST_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <map>

#include "../message_io_stream.h"

#include "message/message_options.h"

namespace dsa {
class Variant;

class OutgoingListStream : public MessageRefedStream {
 public:
  typedef std::function<void(OutgoingListStream &)> CancelCallback;

 protected:
  CancelCallback _cancel_callback;

  void close_impl() override;

  std::map<std::string, std::vector<uint8_t>> _cached_map;
  size_t _next_size;

 public:
  OutgoingListStream(ref_<Session> &&session, const Path &path, uint32_t rid,
                     ListOptions &&options);

  void update_value(const std::string &key, std::vector<uint8_t> &&value);
  void update_value(const std::string &key, const Variant &v);

  size_t peek_next_message_size(size_t available) override;
  MessageCRef get_next_message(AckCallback &) override;

  void receive_message(MessageCRef &&mesage) override;
};
}

#endif  // DSA_SDK_OUTGOING_LIST_STREAM_H
