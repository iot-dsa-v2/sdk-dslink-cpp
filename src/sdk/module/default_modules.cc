#include "dsa_common.h"

#include "default_modules.h"

#include "core/app.h"
#include "crypto/ecdh.h"
#include "default/simple_security_manager.h"

namespace dsa {
DefaultModules::DefaultModules(App &app)
    : ecdh(new ECDH), strand(app.new_strand()), security_manager(make_intrusive_<SimpleSecurityManager>()) {}

DefaultModules::DefaultModules(App &app, boost::asio::io_service::strand &strand)
    : ecdh(new ECDH), strand(strand),
      security_manager(make_intrusive_<AsyncSimpleSecurityManager>(strand)) {}

Config DefaultModules::get_config() {
  Config config(ecdh, strand);

  config.security_manager = security_manager;

  return config;
}
}  // namespace dsa
