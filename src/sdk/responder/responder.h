#ifndef DSA_SDK_RESPONDER_RESPONDER_H
#define DSA_SDK_RESPONDER_RESPONDER_H

#include <map>
#include <string>

#include "core/message_stream.h"
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
  SecurityManager *_security_manager;
  NodeModelManager *_model_manager;
  NodeStateManager _state_manager;
  boost::asio::strand &_strand;
  std::atomic_size_t _stream_count{0};

  std::map< uint32_t, intrusive_ptr_<MessageStream> > _outgoing_streams;

  virtual void on_invoke_request(InvokeRequestMessage &request);
  virtual void on_list_request(ListRequestMessage &request);
  virtual void on_set_request(SetRequestMessage &request);
  virtual void on_subscribe_request(SubscribeRequestMessage &request);

  void send_error(MessageType &&type, MessageStatus &&status, uint32_t &&request_id = 0);

 public:
  Responder(Session &session, const Config &config);

  void receive_message(intrusive_ptr_<RequestMessage> message);
};

}  // namespace dsa

#endif  // DSA_SDK_RESPONDER_RESPONDER_H
