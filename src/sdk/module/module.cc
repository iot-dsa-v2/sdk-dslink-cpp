#include "dsa_common.h"

#include "module.h"

#include "authorizer.h"
#include "client_manager.h"
#include "logger.h"
#include "storage.h"
#include "web_server/login_manager.h"

namespace dsa {

// These are default functions that no effect on the module
Module::Module() {
  _storage = nullptr;
  _logger = nullptr;
  _client_manager = nullptr;
  _authorizer = nullptr;
  _login_manager = nullptr;
}

void Module::init_all(App& app, const LinkStrandRef& strand) {
  init_storage(app, strand);
  init_logger(app, strand);
  init_client_manager(app, strand);
  init_authorizer(app, strand);
  init_login_manager(app, strand);
}

void Module::init_storage(App& app, const LinkStrandRef& strand) {
  if (_storage == nullptr) _storage = create_storage(app, strand);
}

void Module::init_logger(App& app, const LinkStrandRef& strand) {
  if (_logger == nullptr) _logger = create_logger(app, strand);
}

void Module::init_client_manager(App& app, const LinkStrandRef& strand) {
  if (_client_manager == nullptr)
    _client_manager = create_client_manager(app, strand);
}

void Module::init_authorizer(App& app, const LinkStrandRef& strand) {
  if (_authorizer == nullptr) _authorizer = create_authorizer(app, strand);
}

void Module::init_login_manager(App& app, const LinkStrandRef& strand) {
  if (_login_manager == nullptr)
    _login_manager = create_login_manager(app, strand);
}

ref_<Storage> Module::create_storage(App& app, const LinkStrandRef& strand) {
  return nullptr;
}
shared_ptr_<Logger> Module::create_logger(App& app,
                                          const LinkStrandRef& strand) {
  return nullptr;
}
ref_<ClientManager> Module::create_client_manager(App& app,
                                                  const LinkStrandRef& strand) {
  return nullptr;
}
ref_<Authorizer> Module::create_authorizer(App& app,
                                           const LinkStrandRef& strand) {
  return nullptr;
}
shared_ptr_<LoginManager> Module::create_login_manager(
    App& app, const LinkStrandRef& strand) {
  return nullptr;
}

ref_<Storage> Module::get_storage() { return _storage; }
shared_ptr_<Logger> Module::get_logger() { return _logger; }
ref_<ClientManager> Module::get_client_manager() { return _client_manager; }
ref_<Authorizer> Module::get_authorizer() { return _authorizer; }
shared_ptr_<LoginManager> Module::get_login_manager() { return _login_manager; }

void Module::add_module_node(NodeModel& module_node, BrokerPubRoot& pub_root) {}
void Module::add_web_handler() {}

void Module::destroy_impl() {
  if (_storage != nullptr) _storage->destroy();
  if (_client_manager != nullptr) _client_manager->destroy();
  if (_authorizer != nullptr) _authorizer->destroy();
  if (_logger != nullptr) _logger->destroy();
  if (_login_manager != nullptr) _login_manager->destroy();

  DestroyableRef::destroy_impl();
}
}