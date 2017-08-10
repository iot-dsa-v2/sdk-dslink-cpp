#include "dsa_common.h"

#include "default_modules.h"

#include "crypto/ecdh.h"
#include "default/simple_security_manager.h"

namespace dsa {
DefaultModules::DefaultModules()
    : ecdh(new ECDH), security_manager(new SimpleSecurityManager()) {}
DefaultModules::DefaultModules(boost::asio::io_service::strand& strand)
    : ecdh(new ECDH),
      security_manager(new AsyncSimpleSecurityManager(strand)) {}
Config DefaultModules::get_config() {
  Config config(ecdh.get());

  config.security_manager = security_manager.get();

  return std::move(config);
}
}  // namespace dsa
