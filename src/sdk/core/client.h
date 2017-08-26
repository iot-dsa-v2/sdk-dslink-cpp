#ifndef DSA_SDK_CORE_CLIENT_H_
#define DSA_SDK_CORE_CLIENT_H_

#include "config.h"
#include "session.h"
#include "util/enable_shared.h"

namespace dsa {

class Connection;

class Client : public SharedClosable<Client> {
 protected:
  // for Session/Requester/Responder

  // for ClientConnection
  LinkStrandPtr _strand;
  uint32_t handshake_timeout_ms;
  std::string _dsid_prefix;
  std::string _client_token;

  shared_ptr_<Connection> _connection;
  intrusive_ptr_<Session> _session;

  void close_impl() override;

 public:
  Client(WrapperConfig &config);

  LinkStrand &get_strand() const { return *_strand; }
  uint32_t get_handshake_timeout_ms() const { return handshake_timeout_ms; }
  const std::string &get_dsid_prefix() const { return _dsid_prefix; }
  const std::string &get_client_token() const { return _client_token; }

  virtual void connect() = 0;
  
};

}  // namespace dsa

#endif  // DSA_SDK_CORE_CLIENT_H_
