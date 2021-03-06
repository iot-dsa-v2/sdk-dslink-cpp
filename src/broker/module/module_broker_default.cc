#include "dsa_common.h"

#include "module_broker_default.h"

#include "authorizer/broker_authorizer.h"
#include "client/broker_client_manager.h"
#include "module/default/console_logger.h"
#include "module/default/simple_storage.h"

#include "responder/node_model.h"

namespace dsa {

ref_<Storage> ModuleBrokerDefault::create_storage(App &app,
                                                  const LinkStrandRef &strand) {
  return make_ref_<SimpleStorage>(&app.io_service());
}

shared_ptr_<Logger> ModuleBrokerDefault::create_logger(
    App &app, const LinkStrandRef &strand) {
  return Logger::_().shared_from_this();
}

ref_<ClientManager> ModuleBrokerDefault::create_client_manager(
    App &app, const LinkStrandRef &strand) {
  return make_ref_<BrokerClientManager>(strand);
}

ref_<Authorizer> ModuleBrokerDefault::create_authorizer(
    App &app, const LinkStrandRef &strand) {
  return make_ref_<BrokerAuthorizer>(strand);
}

void ModuleBrokerDefault::add_module_node(NodeModel &module_node,
                                          BrokerPubRoot &pub_root) {
  if (_client_manager != nullptr) {
    auto &client_manager = static_cast<BrokerClientManager &>(*_client_manager);
    client_manager.create_nodes(module_node, pub_root);
    module_node.add_list_child("clients", client_manager.get_clients_root());
    module_node.add_list_child("quarantine",
                               client_manager.get_quarantine_root());
    module_node.add_list_child("tokens", client_manager.get_tokens_root());
  }
  if (_authorizer != nullptr) {
    auto &authorizer = static_cast<BrokerAuthorizer &>(*_authorizer);
    authorizer.create_nodes(module_node, pub_root);
    module_node.add_list_child("roles", authorizer.get_permission_root());
  }
}
}  // namespace dsa
