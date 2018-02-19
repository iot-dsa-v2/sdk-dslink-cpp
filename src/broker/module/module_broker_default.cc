#include "dsa_common.h"

#include "module_broker_default.h"

#include "broker_authorizer.h"
#include "broker_client_manager.h"
#include "broker_login_manager.h"
#include "module/default/console_logger.h"
#include "module/default/simple_storage.h"

namespace dsa {

ref_<Storage> ModuleBrokerDefault::create_storage(App& app,
                                                  ref_<LinkStrand> strand) {
  return make_ref_<SimpleStorage>(&app.io_service());
}

shared_ptr_<Logger> ModuleBrokerDefault::create_logger(
    App& app, ref_<LinkStrand> strand) {
  return make_shared_<ConsoleLogger>();
}

ref_<ClientManager> ModuleBrokerDefault::create_client_manager(
    App& app, ref_<LinkStrand> strand) {
  return make_ref_<BrokerClientManager>();
}

ref_<Authorizer> ModuleBrokerDefault::create_authorizer(
    App& app, ref_<LinkStrand> strand) {
  return make_ref_<BrokerAuthorizer>();
}

shared_ptr_<LoginManager> ModuleBrokerDefault::create_login_manager(
    App& app, ref_<LinkStrand> strand) {
  return make_shared_<BrokerLoginManager>(strand);
}

void ModuleBrokerDefault::add_module_node(ref_<NodeModel>& module_node) {
  if (_client_manager != nullptr) {
  }
  if (_authorizer != nullptr) {
  }
}
}
