#include "dsa_common.h"

#include "broker.h"


#include "module/default/simple_security.h"
#include "module/default/simple_session_manager.h"

#include "config/broker_config.h"

#include "module/default/console_logger.h"
#include "module/broker_client_manager.h"
#include "module/broker_authorizer.h"

#include "module/module_broker_default.h"



using namespace dsa;

int main(int argc, const char* argv[]) {
  ref_<BrokerConfig> broker_config = make_ref_<BrokerConfig>(argc, argv);
  auto broker = make_ref_<DsBroker>(std::move(broker_config), make_ref_<ModuleBrokerDefault>());
  broker->run();
  return 0;
}