#ifndef DSA_SDK_RESPONDER_RESPONDER_H
#define DSA_SDK_RESPONDER_RESPONDER_H

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
  Responder(const std::shared_ptr<App> &app, Config config);
  Responder(Config config);
  ~Responder() override;

  ////////////////////////////////////////////
  // Member Functions
  ////////////////////////////////////////////
  void start();
  void stop() override;
  virtual void on_invoke_request(InvokeRequestMessage &request) = 0;
  virtual void on_list_request(ListRequestMessage &request) = 0;
  virtual void on_set_request(SetRequestMessage &request) = 0;
  virtual void on_subscribe_request(SubscribeRequestMessage &request) = 0;

 private:
  ////////////////////////////////////////////
  // Private Members
  ////////////////////////////////////////////
  std::unique_ptr<Client> _connection;
  NodeStateManager _state_manager;
  NodeModelManager _model_manager;

  void _message_handler(const std::shared_ptr<Session> &session, Buffer::SharedBuffer buf);
};

}  // namespace dsa

#endif  // DSA_SDK_RESPONDER_RESPONDER_H
