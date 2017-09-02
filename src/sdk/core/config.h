#ifndef DSA_SDK_CONFIG_H_
#define DSA_SDK_CONFIG_H_

#include "link_strand.h"
#include "module/logger.h"
#include "module/security_manager.h"
#include "responder/node_state_manager.h"
#include "session_manager.h"

namespace dsa {

class LinkConfig : public LinkStrand {
 protected:
  // modules
  shared_ptr_<SecurityManager> _security_manager = nullptr;
  shared_ptr_<OutgoingStreamAcceptor> _stream_acceptor = nullptr;
  shared_ptr_<SessionManager> _session_manager = nullptr;
  shared_ptr_<Logger> _logger = nullptr;

 public:
  explicit LinkConfig(AsioStrand* strand, ECDH* ecdh);

  void set_security_manager(shared_ptr_<SecurityManager> p) {
    __security_manager = p.get();
    _security_manager = std::move(p);
  };
  void set_stream_acceptor(shared_ptr_<OutgoingStreamAcceptor> p) {
    __stream_acceptor = p.get();
    _stream_acceptor = std::move(p);
  };
  void set_session_manager(shared_ptr_<SessionManager> p) {
    __session_manager = p.get();
    _session_manager = std::move(p);
  };

  void set_logger(shared_ptr_<Logger> p) {
    __logger = p.get();
    _logger = std::move(p);
  };
};

class WrapperConfig {
 public:
  LinkStrandRef strand;
  std::string dsid_prefix;
  std::string tcp_host;
  uint16_t tcp_port{0};

  uint32_t handshake_timeout_ms = 5000;

  // client configs
  std::string client_token;
};

}  // namespace dsa

#endif  // DSA_SDK_CONFIG_H_
