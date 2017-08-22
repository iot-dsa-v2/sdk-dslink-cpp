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

  // for ServerConnection
  LinkStrandPtr _strand;
  uint32_t _handshake_timeout_ms;
  std::string _dsid_prefix;

  //  void on_session_connected(const intrusive_ptr_ <Session> session);

 public:
  enum Protocol { TCP };

  explicit Server(WrapperConfig &config);

  LinkStrand &get_strand() const { return *_strand; }
  uint32_t get_handshake_timeout_ms() const { return _handshake_timeout_ms; }
  const std::string &get_dsid_prefix() const { return _dsid_prefix; }

  virtual void start() = 0;
  void close() override;
  virtual std::string type() = 0;
};

typedef shared_ptr_<Server> ServerPtr;

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_SERVER_H
