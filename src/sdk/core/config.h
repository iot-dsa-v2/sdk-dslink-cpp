#ifndef DSA_SDK_CONFIG_H_
#define DSA_SDK_CONFIG_H_

#include <vector>

#include <boost/asio.hpp>

#include "crypto/ecdh.h"
#include "module/security_manager.h"
#include "responder/node_model_manager.h"
#include "responder/node_state_manager.h"

namespace dsa {
struct Config {
  // modules
  intrusive_ptr_<SecurityManager> security_manager;
  intrusive_ptr_<NodeModelManager> model_manager;
  intrusive_ptr_<NodeStateManager> state_manager;
  boost::asio::io_service::strand &strand;

  // shared by both server and client
  std::string dsid_prefix;
  intrusive_ptr_<ECDH> ecdh{nullptr};

  std::string tcp_host;
  uint16_t tcp_port = 0;

  uint32_t handshake_timeout_ms = 5000;

  // client configs
  std::string client_token;

  Config(intrusive_ptr_<ECDH> ecdh, boost::asio::io_service::strand &strand);
};

}  // namespace dsa

#endif  // DSA_SDK_CONFIG_H_
