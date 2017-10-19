#ifndef DSA_SDK_REQUESTER_H_
#define DSA_SDK_REQUESTER_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>

#include "stream/requester/incoming_invoke_stream.h"
#include "stream/requester/incoming_list_stream.h"
#include "stream/requester/incoming_subscribe_stream.h"
#include "stream/requester/incoming_set_stream.h"

namespace dsa {

class Session;

class Requester {
  friend class Session;

 protected:
  Session &_session;
  uint32_t _next_rid = 0;
  uint32_t next_rid();

  std::unordered_map<uint32_t, ref_<MessageStream>> _incoming_streams;

  void receive_message(MessageRef &&message);

  void close_impl();

 public:
  explicit Requester(Session &session);

  ref_<IncomingSubscribeStream> subscribe(
      const std::string &path, IncomingSubscribeStream::Callback &&callback,
      const SubscribeOptions &options =
          IncomingSubscribeStream::default_options);

  ref_<IncomingListStream> list(
      const std::string &path, IncomingListStream::Callback &&callback,
      const ListOptions &options = IncomingListStream::default_options);

  ref_<IncomingInvokeStream> invoke(const std::string &path,
                                  IncomingInvokeStream::Callback &&callback,
                                  ref_<const InvokeRequestMessage> &&message);

  ref_<IncomingSetStream> set(const std::string &path,
                                    IncomingSetStream::Callback &&callback,
                                    ref_<const SetRequestMessage> &&message);
};

}  // namespace dsa

#endif  // DSA_SDK_REQUESTER_H_
