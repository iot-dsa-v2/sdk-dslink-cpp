#ifndef DSA_SDK_RESPONDER_RESPONDER_H
#define DSA_SDK_RESPONDER_RESPONDER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <string>
#include <unordered_map>

#include "node_state_manager.h"

namespace dsa {
class Session;
class SecurityManager;
class NodeStateManager;
class SubscribeRequestMessage;
class InvokeRequestMessage;
class SetRequestMessage;
class ListRequestMessage;

class Responder {
  friend class Session;

 protected:
  Session &_session;

  std::unordered_map<int32_t, ref_<MessageStream> > _outgoing_streams;

  ref_<OutgoingInvokeStream> on_invoke_request(
      ref_<InvokeRequestMessage> &&request);
  ref_<OutgoingListStream> on_list_request(ref_<ListRequestMessage> &&request);
  ref_<OutgoingSetStream> on_set_request(ref_<SetRequestMessage> &&request);
  ref_<OutgoingSubscribeStream> on_subscribe_request(
      ref_<SubscribeRequestMessage> &&request);

  void receive_message(ref_<Message> &&message);

  void destroy_impl();

  // temporary disconnection, might be reconnected
  // TODO: void disconnected();

  // previous connection is lost
  // even the reconnection happens, it won't reuse cached stream
  void connection_changed();

 public:
  explicit Responder(Session &session);

  bool destroy_stream(int32_t rid);
};

}  // namespace dsa

#endif  // DSA_SDK_RESPONDER_RESPONDER_H
