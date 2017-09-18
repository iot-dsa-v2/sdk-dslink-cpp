#ifndef DSA_SDK_CONFIG_H_
#define DSA_SDK_CONFIG_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "link_strand.h"
#include "module/logger.h"
#include "module/security_manager.h"
#include "responder/node_state_manager.h"
#include "session_manager.h"

namespace dsa {

class LinkConfig : public LinkStrand {
 protected:
  // modules
  std::unique_ptr<SecurityManager> _security_manager = nullptr;
  std::unique_ptr<OutgoingStreamAcceptor> _stream_acceptor = nullptr;
  std::unique_ptr<SessionManager> _session_manager = nullptr;
  std::unique_ptr<Logger> _logger = nullptr;

 public:
  explicit LinkConfig(AsioStrand* strand, ECDH* ecdh);

  void set_security_manager(std::unique_ptr<SecurityManager> p) {
    __security_manager = p.get();
    _security_manager = std::move(p);
  };
  void set_stream_acceptor(std::unique_ptr<OutgoingStreamAcceptor> p) {
    __stream_acceptor = p.get();
    _stream_acceptor = std::move(p);
  };
  void set_session_manager(std::unique_ptr<SessionManager> p) {
    __session_manager = p.get();
    _session_manager = std::move(p);
  };

  void set_logger(std::unique_ptr<Logger> p) {
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
