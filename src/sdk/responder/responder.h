#ifndef DSA_SDK_RESPONDER_RESPONDER_H
#define DSA_SDK_RESPONDER_RESPONDER_H

#include <string>

#include "dsa/network.h"
#include "dsa/message.h"
#include "node_state_manager.h"
#include "node_model_manager.h"

namespace dsa {

// Abstract class for a responder DSLink
class Responder : virtual public GracefullyClosable {
 public:
  ////////////////////////////////////////////
  // Config
  ////////////////////////////////////////////
  struct Config {
    unsigned short listen_port{8080};
    unsigned short broker_port{8081};
    std::string broker_hostname{"127.0.0.1"};
    Server::Protocol protocol{Server::TCP};
  };

  ////////////////////////////////////////////
  // Constructors & Destructors
  ////////////////////////////////////////////
  Responder();
  Responder(Config config);
  ~Responder() override;

  ////////////////////////////////////////////
  // Member Functions
  ////////////////////////////////////////////
  void start();
  virtual void stop();
  virtual void on_invoke_request(InvokeRequestMessage request) = 0;
  virtual void on_list_request(ListRequestMessage request) = 0;
  virtual void on_set_request(SetRequestMessage request) = 0;
  virtual void on_subscribe_request(SubscribeRequestMessage request) = 0;

 private:
  ////////////////////////////////////////////
  // Private Members
  ////////////////////////////////////////////
  std::unique_ptr<Client> _connection;
  NodeStateManager _state_manager;
  NodeModelManager _model_manager;
};

}  // namespace dsa

#endif  // DSA_SDK_RESPONDER_RESPONDER_H
