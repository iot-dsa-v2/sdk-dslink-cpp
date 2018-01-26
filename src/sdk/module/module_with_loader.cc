#include "dsa_common.h"

#include "module_with_loader.h"

#include "storage.h"
#include "logger.h"
#include "client_manager.h"
#include "authorizer.h"


#include <boost/dll/import.hpp>


namespace dsa{

ModuleWithLoader::ModuleWithLoader(bf::path lib_path, ref_<Module> &&default_module) {
  _default_module = std::move(default_module);

  auto module_suffix = boost::dll::shared_library::suffix();

  // There is no module directory returning default function
  if (!bf::is_directory(lib_path)) return;

  // Iterate directory files
  std::vector<bf::path> module_paths;
  bf::directory_iterator end_itr;

  for (bf::directory_iterator itr(lib_path); itr != end_itr; ++itr) {
    if (bf::is_regular_file(itr->path())) {
      auto file_name = itr->path().filename().string();
      auto file_suffix = itr->path().extension();

      // check extension
      if (file_suffix == module_suffix) {
        module_paths.push_back(itr->path());
      }
    }
  }

  // TODO: create Module objects

//  // we found our shared lib, get create function from it
//  try {
//    return boost::dll::import_alias<T>(shared_library_path, function_name);
//  } catch (int e) {
//    std::cout << "Exception while loading " << module_name << std::endl;
//  }
}


ref_<Storage> ModuleWithLoader::create_storage(App& app, ref_<LinkStrand> strand){
  ref_<Storage> service = nullptr;

  // Check in files
  for(auto module:_modules){
    module->init_storage(app,strand);
    auto temp = module->get_storage();
    if(temp == nullptr) continue;

    if(service == nullptr) {
      service = temp;
    }
    else{
      LOG_FATAL("There are more than one storage in libs directory, cannot select them!")
    }
  }

  if(service != nullptr) return service;

  _default_module->init_storage(app,strand);
  return _default_module->get_storage();
}

ref_<Logger> ModuleWithLoader::create_logger(App& app, ref_<LinkStrand> strand){
  ref_<Logger> service = nullptr;

  // Check in files
  for(auto module:_modules){
    module->init_logger(app,strand);
    auto temp = module->get_logger();
    if(temp == nullptr) continue;

    if(service == nullptr) {
      service = temp;
    }
    else{
      LOG_FATAL("There are more than one logger in libs directory, cannot select them!")
    }
  }

  if(service != nullptr) return service;

  _default_module->init_logger(app,strand);
  return _default_module->get_logger();
}

ref_<ClientManager> ModuleWithLoader::create_client_manager(App& app, ref_<LinkStrand> strand){
  ref_<ClientManager> service = nullptr;

  // Check in files
  for(auto module:_modules){
    module->init_client_manager(app,strand);
    auto temp = module->get_client_manager();
    if(temp == nullptr) continue;

    if(service == nullptr) {
      service = temp;
    }
    else{
      LOG_FATAL("There are more than one client manager in libs directory, cannot select them!")
    }
  }

  if(service != nullptr) return service;

  _default_module->init_client_manager(app,strand);
  return _default_module->get_client_manager();
}

ref_<Authorizer> ModuleWithLoader::create_authorizer(App& app, ref_<LinkStrand> strand){
  ref_<Authorizer> service = nullptr;

  // Check in files
  for(auto module:_modules){
    module->init_authorizer(app,strand);
    auto temp = module->get_authorizer();
    if(temp == nullptr) continue;

    if(service == nullptr) {
      service = temp;
    }
    else{
      LOG_FATAL("There are more than one authorizer in libs directory, cannot select them!")
    }
  }

  if(service != nullptr) return service;

  _default_module->init_authorizer(app,strand);
  return _default_module->get_authorizer();
}


void ModuleWithLoader::add_module_node() {
  _default_module->add_module_node();
  for(auto module:_modules){
    module->add_module_node();
  }
}

void ModuleWithLoader::add_web_handler() {
  _default_module->add_module_node();
  for(auto module:_modules){
    module->add_web_handler();
  }
}

}
