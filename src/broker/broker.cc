#include "dsa_common.h"

#include "broker.h"

#include "config/broker_config.h"
#include "config/module_loader.h"

namespace dsa {
void DsBroker::destroy_impl() {}
}

using namespace dsa;

int main(int argc, const char* argv[]) {
  ref_<BrokerConfig> broker_config = make_ref_<BrokerConfig>(argc, argv);
  ModuleLoader loader(broker_config);

  return 0;
}