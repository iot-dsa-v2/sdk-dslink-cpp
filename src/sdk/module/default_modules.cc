#include "dsa_common.h"

#include "default_modules.h"

#include "core/app.h"
#include "default/console_logger.h"
#include "default/simple_security_manager.h"

namespace dsa {

DefaultModules::DefaultModules(App &app, bool async)
    : LinkConfig(app.new_strand(), new ECDH()) {
  LinkStrandRef strand = get_ref();
  set_session_manager(make_shared_<SessionManager>(strand));
  set_stream_acceptor(make_shared_<NodeStateManager>(strand));

  if (async) {
    set_security_manager(make_shared_<AsyncSimpleSecurityManager>(__strand));
  } else {
    set_security_manager(make_shared_<SimpleSecurityManager>());
  }

  set_logger(make_shared_<ConsoleLogger>());
}

}  // namespace dsa
