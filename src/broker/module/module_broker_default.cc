#include "dsa_common.h"

#include "module_broker_default.h"

#include "module/default/simple_storage.h"
#include "module/default/console_logger.h"
#include "broker_client_manager.h"
#include "broker_authorizer.h"

namespace dsa {

  ref_<Storage> ModuleBrokerDefault::create_storage(App& app, ref_<LinkStrand> strand){
    //return make_ref_<SimpleStorage>(&app.io_service());
    return nullptr;
  }

  ref_<Logger> ModuleBrokerDefault::create_logger(App& app, ref_<LinkStrand> strand){
    return make_ref_<ConsoleLogger>();
  }

  ref_<ClientManager> ModuleBrokerDefault::create_client_manager(App& app, ref_<LinkStrand> strand){
    return make_ref_<BrokerClientManager>();
  }

  ref_<Authorizer> ModuleBrokerDefault::create_authorizer(App& app, ref_<LinkStrand> strand){
    return make_ref_<BrokerAuthorizer>();
  }

}
