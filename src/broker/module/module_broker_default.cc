#include "dsa_common.h"

#include "module_broker_default.h"

#include "broker_authorizer.h"
#include "broker_client_manager.h"
#include "broker_login_manager.h"
#include "module/default/console_logger.h"
#include "module/default/simple_storage.h"

#include "responder/node_model.h"

namespace dsa {

ref_<Storage> ModuleBrokerDefault::create_storage(App& app,
                                                  ref_<LinkStrand>& strand) {
  return make_ref_<SimpleStorage>(&app.io_service());
}

shared_ptr_<Logger> ModuleBrokerDefault::create_logger(
    App& app, ref_<LinkStrand>& strand) {
  return make_shared_<ConsoleLogger>();
}

ref_<ClientManager> ModuleBrokerDefault::create_client_manager(
    App& app, ref_<LinkStrand>& strand) {
  return make_ref_<BrokerClientManager>(strand);
}

ref_<Authorizer> ModuleBrokerDefault::create_authorizer(
    App& app, ref_<LinkStrand>& strand) {
  return make_ref_<BrokerAuthorizer>(strand);
}

shared_ptr_<LoginManager> ModuleBrokerDefault::create_login_manager(
    App& app, ref_<LinkStrand>& strand) {
  return make_shared_<BrokerLoginManager>(strand);
}

void ModuleBrokerDefault::add_module_node(NodeModel& module_node,
                                          BrokerPubRoot& pub_root) {
  if (_client_manager != nullptr) {
    static_cast<BrokerClientManager&>(*_client_manager)
        .create_nodes(module_node, pub_root);
    module_node.add_list_child(
        "Clients", static_cast<BrokerClientManager &>(*_client_manager)
            .get_clients_root());
    module_node.add_list_child(
        "Quarantine", static_cast<BrokerClientManager &>(*_client_manager)
            .get_quarantine_root());
  }
  if (_authorizer != nullptr) {
  }
  if (_login_manager != nullptr) {
//    module_node.add_list_child(
//        "Users", static_cast<BrokerLoginManager&>(*_login_manager)._module_node);
  }
}
}  // namespace dsa
