#include "dsa_common.h"

#include "module.h"

#include "storage.h"
#include "logger.h"
#include "client_manager.h"
#include "authorizer.h"

namespace dsa {

// These are default functions that no effect on the module
Module::Module(){
  _storage = nullptr;
  _logger = nullptr;
  _client_manager = nullptr;
  _authorizer = nullptr;
}

void Module::init_all(App& app, ref_<LinkStrand> strand){
  init_storage(app, strand);
  init_logger(app, strand);
  init_client_manager(app, strand);
  init_authorizer(app, strand);
}

void Module::init_storage(App& app, ref_<LinkStrand> strand){
  if(_storage == nullptr) _storage = create_storage(app, strand);
}

void Module::init_logger(App& app, ref_<LinkStrand> strand){
  if(_logger == nullptr) _logger = create_logger(app, strand);
}

void Module::init_client_manager(App& app, ref_<LinkStrand> strand){
  if(_client_manager == nullptr) _client_manager = create_client_manager(app, strand);
}

void Module::init_authorizer(App& app, ref_<LinkStrand> strand){
  if(_authorizer == nullptr) _authorizer = create_authorizer(app, strand);
}

ref_<Storage> Module::create_storage(App &app, ref_<LinkStrand> strand) {return nullptr;}
shared_ptr_<Logger> Module::create_logger(App &app, ref_<LinkStrand> strand) {return nullptr;}
ref_<ClientManager> Module::create_client_manager(App &app, ref_<LinkStrand> strand) {return nullptr;}
ref_<Authorizer> Module::create_authorizer(App &app, ref_<LinkStrand> strand) {return nullptr;}

ref_<Storage> Module::get_storage(){ return _storage; }
shared_ptr_<Logger> Module::get_logger(){ return _logger; }
ref_<ClientManager> Module::get_client_manager(){ return _client_manager; }
ref_<Authorizer> Module::get_authorizer(){ return _authorizer; }

void Module::add_module_node(){}
void Module::add_web_handler(){}

}