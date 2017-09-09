#include "dsa_common.h"

#include "default_modules.h"

#include "core/app.h"
#include "default/console_logger.h"
#include "default/simple_security_manager.h"

namespace dsa {

DefaultModules::DefaultModules(App &app, bool async)
    : LinkConfig(app.new_strand(), new ECDH()) {
  set_session_manager(std::make_unique<SessionManager>(this));
  set_stream_acceptor(std::make_unique<NodeStateManager>());
  if (async) {
    set_security_manager(std::make_unique<AsyncSimpleSecurityManager>(__strand));
  } else {
    set_security_manager(std::make_unique<SimpleSecurityManager>());
  }

  set_logger(std::make_unique<ConsoleLogger>());
}

}  // namespace dsa
