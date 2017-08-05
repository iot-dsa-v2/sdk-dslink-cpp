#ifndef DSA_SDK_RESPONDER_RESPONDER_H
#define DSA_SDK_RESPONDER_RESPONDER_H

#include <atomic>
#include <string>

#include "util/enable_shared.h"


namespace dsa {

class Responder {
 public:
  Responder(){};
};

/*
class InvokeRequestMessage;
class ListRequestMessage;
class SetRequestMessage;
class SubscribeRequestMessage;

// Abstract class for a responder DSLink
class Responder : public GracefullyClosable<Responder> {
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
  Responder(const App &app, Config config);
  Responder(Config config);
  ~Responder();

  ////////////////////////////////////////////
  // Member Functions
  ////////////////////////////////////////////
  void start();
  void close() override;
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

  void _message_handler(const intrusive_ptr_<Session> &session,
Buffer::SharedBuffer buf);
};
*/

}  // namespace dsa

#endif  // DSA_SDK_RESPONDER_RESPONDER_H
