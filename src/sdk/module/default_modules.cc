#include "dsa_common.h"

#include "default_modules.h"

namespace dsa {
DefaultModules::DefaultModules() {}
Config DefaultModules::get_config() {
  Config config(&ecdh);

  config.security_manager = new SimpleSecurityManager();

  return std::move(config);
}
}  // namespace dsa
