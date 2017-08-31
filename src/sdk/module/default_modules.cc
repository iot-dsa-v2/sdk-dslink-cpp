#include "dsa_common.h"

#include "default_modules.h"

#include "core/app.h"
#include "default/console_logger.h"
#include "default/simple_security_manager.h"

namespace dsa {

DefaultModules::DefaultModules(App &app, bool async)
    : LinkConfig(app.new_strand(), new ECDH()) {
  LinkStrandRef ptr = get_ref();
  set_session_manager(new SessionManager(ptr));
  set_state_manager(new NodeStateManager(ptr));

  if (async) {
    set_security_manager(new AsyncSimpleSecurityManager(__strand));
  } else {
    set_security_manager(new SimpleSecurityManager());
  }

  set_logger(new ConsoleLogger());
}

}  // namespace dsa
