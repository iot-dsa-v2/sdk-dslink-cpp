#ifndef DSA_SDK_RESPONDER_RESPONDER_H
#define DSA_SDK_RESPONDER_RESPONDER_H

#include <atomic>
#include <string>

#include "dsa/network.h"
#include "dsa/message.h"
#include "node_state_manager.h"
#include "node_model_manager.h"

namespace dsa {

// Abstract class for a responder DSLink
class Responder : public GracefullyClosable {
 public:
  ////////////////////////////////////////////
  // Config
  ////////////////////////////////////////////
  struct Config {
    unsigned short broker_port{8081};
    std::string broker_hostname{"127.0.0.1"};
    Client::Protocol protocol{Client::TCP};
  };

  ////////////////////////////////////////////
  // Constructors & Destructors
  ////////////////////////////////////////////
  Responder(const shared_ptr_<App> &app, Config config);
  Responder(Config config);
  ~Responder();

  ////////////////////////////////////////////
  // Member Functions
  ////////////////////////////////////////////
  void start();
  void stop() override;
  virtual void on_invoke_request(InvokeRequestMessage &request);
  virtual void on_list_request(ListRequestMessage &request);
  virtual void on_set_request(SetRequestMessage &request);
  virtual void on_subscribe_request(SubscribeRequestMessage &request);

 private:
  ////////////////////////////////////////////
  // Private Members
  ////////////////////////////////////////////
  std::atomic_size_t _stream_count{0};
  intrusive_ptr_<Session> _session;
  NodeStateManager _state_manager;
  NodeModelManager _model_manager;

  const Config _config;

  ClientPtr _initialize_connection();

  void _message_handler(const intrusive_ptr_<Session> &session, Buffer::SharedBuffer buf);
};

}  // namespace dsa

#endif  // DSA_SDK_RESPONDER_RESPONDER_H
