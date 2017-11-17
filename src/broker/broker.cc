#include "dsa_common.h"

#include "broker.h"

#include "config/broker_config.h"
#include "module/logger.h"

namespace dsa {
DsBroker::DsBroker(ref_<BrokerConfig>&& config, ModuleLoader& modules)
    : _config(std::move(config)) {}
DsBroker::~DsBroker() {}
void DsBroker::run(){

}
}
