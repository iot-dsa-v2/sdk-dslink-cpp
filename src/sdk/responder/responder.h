#ifndef DSA_SDK_RESPONDER_RESPONDER_H
#define DSA_SDK_RESPONDER_RESPONDER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <map>
#include <string>

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

  std::map<uint32_t, ref_<MessageStream> > _outgoing_streams;

  virtual void on_invoke_request(
      ref_<InvokeRequestMessage> &&request);
  virtual void on_list_request(ref_<ListRequestMessage> &&request);
  virtual void on_set_request(ref_<SetRequestMessage> &&request);
  virtual void on_subscribe_request(
      ref_<SubscribeRequestMessage> &&request);

  void receive_message(ref_<Message> &&message);

  void close_impl();
  
 public:
  explicit Responder(Session &session);
};

}  // namespace dsa

#endif  // DSA_SDK_RESPONDER_RESPONDER_H
