#ifndef DSA_SDK_RESPONDER_RESPONDER_H
#define DSA_SDK_RESPONDER_RESPONDER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>
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

  std::unordered_map<int32_t, ref_<MessageStream> > _outgoing_streams;

  void on_invoke_request(
      ref_<InvokeRequestMessage> &&request, PermissionLevel permission_level);
  void on_list_request(ref_<ListRequestMessage> &&request, PermissionLevel permission_level);
  void on_set_request(ref_<SetRequestMessage> &&request, PermissionLevel permission_level);
  void on_subscribe_request(
      ref_<SubscribeRequestMessage> &&request, PermissionLevel permission_level);

  void receive_message(ref_<Message> &&message);

  void destroy_impl();
  
 public:
  explicit Responder(Session &session);

  bool remove_stream(int32_t rid);
};

}  // namespace dsa

#endif  // DSA_SDK_RESPONDER_RESPONDER_H
