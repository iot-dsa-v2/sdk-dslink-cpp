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
  SecurityManager *_security_manager;
  NodeModelManager *_model_manager;
  NodeStateManager _state_manager;
  std::atomic_size_t _stream_count{0};
  boost::asio::strand &_strand;

  virtual void on_invoke_request(InvokeRequestMessage &request);
  virtual void on_list_request(ListRequestMessage &request);
  virtual void on_set_request(SetRequestMessage &request);
  virtual void on_subscribe_request(SubscribeRequestMessage &request);

 public:
  Responder(Session &session, const Config &config);

  void set_ready_stream(MessageStream::StreamInfo &&stream_info);

  void receive_message(intrusive_ptr_<RequestMessage> message);
};

}  // namespace dsa

#endif  // DSA_SDK_RESPONDER_RESPONDER_H
