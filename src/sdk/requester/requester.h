#ifndef DSA_SDK_REQUESTER_H
#define DSA_SDK_REQUESTER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>

#include "message/message_options.h"
#include "stream/stream_callbacks.h"
#include "util/enable_ref.h"

namespace dsa {

class Message;
class Session;
class MessageStream;

class Requester {
  friend class Session;

 protected:
  Session &_session;
  int32_t _next_rid = 0;
  int32_t next_rid();

  std::unordered_map<int32_t, ref_<MessageStream>> _incoming_streams;

  void receive_message(ref_<Message> &&message);

  void destroy_impl();

  void connected();
  // temporary disconnection, might be reconnected
  void disconnected();


 public:
  explicit Requester(Session &session);
  ~Requester();

  ref_<IncomingSubscribeStream> subscribe(
      const string_ &path, IncomingSubscribeStreamCallback &&callback,
      const SubscribeOptions &options = SubscribeOptions::default_options);

  ref_<IncomingListStream> list(
      const string_ &path, IncomingListStreamCallback &&callback,
      const ListOptions &options = ListOptions::default_options);

  ref_<IncomingInvokeStream> invoke(IncomingInvokeStreamCallback &&callback,
                                    ref_<const InvokeRequestMessage> &&message);

  ref_<IncomingSetStream> set(IncomingSetStreamCallback &&callback,
                              ref_<const SetRequestMessage> &&message);

  bool remove_stream(int32_t rid);
};

}  // namespace dsa

#endif  // DSA_SDK_REQUESTER_H
