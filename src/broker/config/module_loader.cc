#include "dsa_common.h"

#include "module_loader.h"

#include "broker_config.h"
#include "module/default/console_logger.h"
#include "module/default/simple_security_manager.h"

namespace dsa {
ModuleLoader::ModuleLoader(ref_<BrokerConfig> config) {}

std::unique_ptr<Logger> ModuleLoader::new_logger(App& app,
                                                 ref_<LinkStrand> strand) {
  return std::unique_ptr<Logger>(new ConsoleLogger());
}
ref_<SecurityManager> ModuleLoader::new_security_manager(
    App& app, ref_<LinkStrand> strand) {
  return make_ref_<AsyncSimpleSecurityManager>(strand);
}
}