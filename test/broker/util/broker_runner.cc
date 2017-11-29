#include "dsa_common.h"

#include "broker_runner.h"
#include "config/broker_config.h"
#include "config/module_loader.h"

namespace dsa {
shared_ptr_<DsBroker> create_broker() {
  const char* empty_argv[1];
  ref_<BrokerConfig> broker_config = make_ref_<BrokerConfig>(0, empty_argv);
  ModuleLoader modules(broker_config);
  return make_shared_<DsBroker>(std::move(broker_config), modules);
}
}
