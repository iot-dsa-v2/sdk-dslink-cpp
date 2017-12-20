#include "dsa_common.h"

#include "broker.h"

#include "config/broker_config.h"
#include "config/module_loader.h"


using namespace dsa;

int main(int argc, const char* argv[]) {
  ref_<BrokerConfig> broker_config = make_ref_<BrokerConfig>(argc, argv);
  ModuleLoader modules(broker_config);
  auto broker = make_ref_<DsBroker>(std::move(broker_config), modules);
  broker->run();
  return 0;
}