#ifndef DSA_SDK_CORE_CLIENT_H_
#define DSA_SDK_CORE_CLIENT_H_

#include <boost/asio.hpp>

#include "config.h"
#include "session.h"
#include "util/enable_shared.h"

namespace dsa {

class ClientConnection;

class Client : public SharedClosable<Client> {
 protected:
  // for Session/Requester/Responder
  intrusive_ptr_<SecurityManager> _security_manager;
  intrusive_ptr_<NodeStateManager> _node_state_manager;
  intrusive_ptr_<NodeModelManager> _node_model_manager;

  // for ClientConnection
  boost::asio::io_service::strand &_strand;
  uint32_t handshake_timeout_ms;
  std::string _dsid_prefix;
  intrusive_ptr_<ECDH> _ecdh;
  std::string _client_token;

  shared_ptr_<ClientConnection> _connection;
  intrusive_ptr_<Session> _session;

 public:
  Client(const Config &config, intrusive_ptr_<Session> &&session = nullptr);

  SecurityManager &get_security_manager() const { return *_security_manager; }
  NodeStateManager &get_node_state_manager() const { return *_node_state_manager; }
  NodeModelManager &get_node_model_manager() const { return *_node_model_manager; }

  boost::asio::io_service::strand &get_strand() const { return _strand; }
  uint32_t get_handshake_timeout_ms() const { return handshake_timeout_ms; }
  const std::string &get_dsid_prefix() const { return _dsid_prefix; }
  const intrusive_ptr_<ECDH> &get_ecdh() const { return _ecdh; }
  const std::string &get_client_token() const { return _client_token; }

  virtual void connect() = 0;
  void close() override;
};

}  // namespace dsa

#endif  // DSA_SDK_CORE_CLIENT_H_
