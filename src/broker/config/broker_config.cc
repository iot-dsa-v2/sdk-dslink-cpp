#include "dsa_common.h"

#include "broker_config.h"

namespace dsa {
BrokerConfig::BrokerConfig(int argc, const char *argv[]) {
  init();
  load();
}

// init all the config properties
void BrokerConfig::init() {}
// load config json from file
void BrokerConfig::load() {}
}
