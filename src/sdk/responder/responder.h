#ifndef DSA_SDK_RESPONDER_RESPONDER_H
#define DSA_SDK_RESPONDER_RESPONDER_H

#include <map>
#include <string>

#include "node_state_manager.h"

namespace dsa {
class Session;
class SecurityManager;
class NodeStateManager;
class NodeModelManager;
class SubscribeRequestMessage;
class InvokeRequestMessage;
class SetRequestMessage;
class ListRequestMessage;
class Config;

class Responder {
  friend class Session;

 protected:
  Session &_session;

  std::map<uint32_t, intrusive_ptr_<MessageStream> > _outgoing_streams;

  virtual void on_invoke_request(
      intrusive_ptr_<InvokeRequestMessage> &&request);
  virtual void on_list_request(intrusive_ptr_<ListRequestMessage> &&request);
  virtual void on_set_request(intrusive_ptr_<SetRequestMessage> &&request);
  virtual void on_subscribe_request(
      intrusive_ptr_<SubscribeRequestMessage> &&request);

  void send_error(MessageType &&type, MessageStatus &&status,
                  uint32_t &&request_id = 0);

 public:
  explicit Responder(Session &session);

  void receive_message(intrusive_ptr_<Message> &&message);
};

}  // namespace dsa

#endif  // DSA_SDK_RESPONDER_RESPONDER_H
