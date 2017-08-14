#ifndef DSA_SDK_RESPONDER_RESPONDER_H
#define DSA_SDK_RESPONDER_RESPONDER_H

#include <map>
#include <string>

#include "outgoing_message_stream.h"

namespace dsa {
class Session;

class Responder {
  friend class Session;

 protected:
  Session &_session;
  std::map<uint32_t, intrusive_ptr_<OutgoingMessageStream>> _outgoing_streams;



 public:
  Responder(Session &session) : _session(session){};

  void add_ready_outgoing_stream(uint32_t rid, size_t unique_id);

  bool add_outgoing_subscription(
      const intrusive_ptr_<OutgoingMessageStream> &stream);

  void remove_outgoing_subscription(uint32_t request_id);

  void receive_message(Message * message);
};

/*
class InvokeRequestMessage;
class ListRequestMessage;
class SetRequestMessage;
class SubscribeRequestMessage;

// Abstract class for a responder DSLink
class Responder : public SharedClosable<Responder> {
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
