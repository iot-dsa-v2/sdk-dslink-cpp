#include "dsa_common.h"

#include "default_modules.h"

#include "core/app.h"
#include "default/simple_security_manager.h"
#include "default/console_logger.h"

namespace dsa {
DefaultModules::DefaultModules(App &app)
    : LinkConfig(app.new_strand(), new ECDH()) {
  LinkStrandPtr ptr = intrusive_this();
  set_session_manager(new SessionManager(ptr));
  set_state_manager(new NodeStateManager(ptr));
  set_security_manager(new SimpleSecurityManager());
  set_logger(new ConsoleLogger());
}

DefaultModules::DefaultModules(App &app, boost::asio::io_service::strand * strand)
  : LinkConfig(app.new_strand(), new ECDH()){
  LinkStrandPtr ptr = intrusive_this();
  set_session_manager(new SessionManager(ptr));
  set_state_manager(new NodeStateManager(ptr));
  set_security_manager(new AsyncSimpleSecurityManager(app.new_strand()));
  set_logger(new ConsoleLogger());
}

}  // namespace dsa
