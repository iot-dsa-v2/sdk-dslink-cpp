#ifndef DSA_SDK_CONFIG_H_
#define DSA_SDK_CONFIG_H_

#include "link_strand.h"
#include "module/security_manager.h"
#include "responder/node_state_manager.h"
#include "session_manager.h"

namespace dsa {

class LinkConfig : public LinkStrand {
  // modules
  std::unique_ptr<SecurityManager> _security_manager{nullptr};
  std::unique_ptr<NodeStateManager> _state_manager{nullptr};
  std::unique_ptr<SessionManager> _session_manager{nullptr};

 public:
  explicit LinkConfig(Strand* strand, ECDH* ecdh);

  void set_security_manager(SecurityManager* p) {
    __security_manager = p;
    _security_manager.reset(p);
  };
  void set_state_manager(NodeStateManager* p) {
    __state_manager = p;
    _state_manager.reset(p);
  };
  void set_session_manager(SessionManager* p) {
    __session_manager = p;
    _session_manager.reset(p);
  };
};

class WrapperConfig {
 public:
  LinkStrandPtr strand;
  std::string dsid_prefix;
  std::string tcp_host;
  uint16_t tcp_port{0};

  uint32_t handshake_timeout_ms = 5000;

  // client configs
  std::string client_token;
};

}  // namespace dsa

#endif  // DSA_SDK_CONFIG_H_
