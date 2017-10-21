#ifndef DSA_SDK_INCOMING_LIST_STREAM_H
#define DSA_SDK_INCOMING_LIST_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../message_io_stream.h"

#include "message/message_options.h"

namespace dsa {

class ListResponseMessage;

class IncomingListStream : public MessageCacheStream {
 public:
  typedef std::function<void(IncomingListStream&,
                             ref_<const ListResponseMessage>&&)>
      Callback;

  static const ListOptions default_options;

 protected:
  Callback _callback;

 public:
  explicit IncomingListStream(ref_<Session>&& session, const Path& path,
                              uint32_t rid, Callback&& callback);
  void receive_message(MessageCRef&& msg) override;

  // send the list request
  void list(const ListOptions& options);

  void close();
};
}

#endif  // DSA_SDK_INCOMING_LIST_STREAM_H
