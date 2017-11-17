#include "dsa_common.h"

#include "broker.h"

#include "config/broker_config.h"
#include "config/module_loader.h"

namespace dsa {
DsBroker::DsBroker(ref_<BrokerConfig>&& config, ModuleLoader& modules)
    : _config(std::move(config)) {
  init();
}
DsBroker::~DsBroker() {

}

void DsBroker::init() {

}
void DsBroker::run() {

}
}
