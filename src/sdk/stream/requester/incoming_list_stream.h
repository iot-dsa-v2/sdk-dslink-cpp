#ifndef DSA_SDK_INCOMING_LIST_STREAM_H
#define DSA_SDK_INCOMING_LIST_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../message_io_stream.h"

#include "message/message_options.h"

namespace dsa {

class ListResponseMessage;

class IncomingListStream final : public MessageCacheStream {
 public:
  typedef std::function<void(IncomingListStream&,
                             ref_<const ListResponseMessage>&&)>
      Callback;

 protected:
  Callback _callback;
  ListOptions _options;

 public:
  explicit IncomingListStream(ref_<Session>&& session, const Path& path,
                              uint32_t rid, Callback&& callback);
  void receive_message(ref_<Message>&& msg) final;

  // send the list request
  void list(const ListOptions& options);

  void close();

  bool check_close_message(MessageCRef& message) final;

  bool connection_changed() final;
  void update_response_status(MessageStatus status = MessageStatus::OK) final;
};
}

#endif  // DSA_SDK_INCOMING_LIST_STREAM_H
