#ifndef DSA_SDK_INCOMING_SUBSCRIBE_STREAM_H
#define DSA_SDK_INCOMING_SUBSCRIBE_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../message_io_stream.h"

#include "message/message_options.h"
#include "message/message_page_group.h"

namespace dsa {

class SubscribeResponseMessage;

class IncomingSubscribeStream final : public MessageCacheStream {
 public:
  typedef std::function<void(IncomingSubscribeStream&,
                             ref_<const SubscribeResponseMessage>&&)>
      Callback;

 protected:
  // paged message that's partially received
  ref_<IncomingPagesMerger> _waiting_pages;
  Callback _callback;
  bool _callback_running = false;
  SubscribeOptions _options;

 public:
  explicit IncomingSubscribeStream(ref_<Session>&& session, const Path& path,
                                   uint32_t rid, Callback&& callback);

  void receive_message(ref_<Message>&& msg) final;

  // send subscription request and update of subscription options
  void subscribe(const SubscribeOptions& options);

  void close();

  bool check_close_message(MessageCRef& message) final;

  bool disconnected() final;
  void reconnected() final;
  void update_response_status(Status status) final;
};
}

#endif  // DSA_SDK_INCOMING_SUBSCRIBE_STREAM_H
