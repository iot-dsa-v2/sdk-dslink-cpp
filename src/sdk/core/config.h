#ifndef DSA_SDK_CONFIG_H_
#define DSA_SDK_CONFIG_H_

#include <vector>

namespace dsa {

class NodeModelManager;
class ECDH;

struct Config {
  // shared by both server and client
  std::string dsid_prefix;
  NodeModelManager* model_manager = nullptr;
  ECDH* ecdh = nullptr;

  std::string tcp_host;
  uint16_t tcp_port = 0;

  uint32_t handshake_timout_ms = 5000;

  // cliend configs
  std::vector<uint8_t> client_token;

  Config();
};

}  // namespace dsa

#endif  // DSA_SDK_CONFIG_H_
