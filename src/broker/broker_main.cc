#include "dsa_common.h"

#include "broker.h"


#include "module/default/simple_security.h"
#include "module/default/simple_session_manager.h"

#include "config/broker_config.h"

#include "module/default/console_logger.h"
#include "module/client/broker_client_manager.h"
#include "module/authorizer/broker_authorizer.h"

#include "module/module_broker_default.h"



using namespace dsa;

int main(int argc, const char* argv[]) {
  ref_<BrokerConfig> broker_config = make_ref_<BrokerConfig>(argc, argv);
  auto broker = make_ref_<DsBroker>(std::move(broker_config));
  broker->run();
  return 0;
}