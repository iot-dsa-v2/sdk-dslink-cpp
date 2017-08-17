#ifndef DSA_SDK_NETWORK_SERVER_H
#define DSA_SDK_NETWORK_SERVER_H

#include <atomic>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <utility>

#include "config.h"
#include "session.h"
#include "session_manager.h"
#include "util/buffer.h"
#include "util/enable_shared.h"

namespace dsa {
class Connection;
class SessionManager;

class Server : public SharedClosable<Server> {
 protected:
  // for Session/Requester/Responder
  SessionManager _session_manager;
  intrusive_ptr_<SecurityManager> _security_manager;
  intrusive_ptr_<NodeStateManager> _node_state_manager;
  intrusive_ptr_<NodeModelManager> _node_model_manager;

  // for ServerConnection
  boost::asio::io_service::strand &_strand;
  uint32_t _handshake_timeout_ms;
  std::string _dsid_prefix;
  intrusive_ptr_<ECDH> _ecdh;

  void on_session_connected(const shared_ptr_<Session> &session);

 public:
  SessionManager &session_manager() { return _session_manager; }

  enum Protocol { TCP };

  explicit Server(const Config &config);

  SecurityManager &get_security_manager() const {
    std::cout << (_security_manager == nullptr) << std::endl;
    return *_security_manager;
  }
  NodeStateManager &get_node_state_manager() const { return *_node_state_manager; }
  NodeModelManager &get_node_model_manager() const { return *_node_model_manager; }

  boost::asio::io_service::strand &get_strand() const { return _strand; }
  uint32_t get_handshake_timeout_ms() const { return _handshake_timeout_ms; }
  const std::string &get_dsid_prefix() const { return _dsid_prefix; }
  const intrusive_ptr_<ECDH> &get_ecdh() const { return _ecdh; }

  virtual void start() = 0;
  void close() override;
  virtual std::string type() = 0;
};

typedef shared_ptr_<Server> ServerPtr;

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_SERVER_H
