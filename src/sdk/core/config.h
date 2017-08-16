#ifndef DSA_SDK_CONFIG_H_
#define DSA_SDK_CONFIG_H_

#include <vector>

namespace dsa {

class NodeModelManager;
class SecurityManager;
class ECDH;

class Config {
public:
  // modules
  NodeModelManager* model_manager = nullptr;
  SecurityManager* security_manager = nullptr;

  // shared by both server and client
  std::string dsid_prefix;
  const ECDH* ecdh = nullptr;

  std::string tcp_host;
  uint16_t tcp_port = 0;

  uint32_t handshake_timeout_ms = 5000;

  // client configs
  std::string client_token;

  Config(const ECDH* ecdh);
};

}  // namespace dsa

#endif  // DSA_SDK_CONFIG_H_
