#include "dsa_common.h"

#include "dsa/util.h"

#include "module_loader.h"

#include "module/default/console_logger.h"
#include "module/default/simple_security_manager.h"
#include "module/default/simple_storage.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/dll/import.hpp>

#include <iostream>

namespace bf = boost::filesystem;

namespace dsa {

boost::function<api_creators_func::security_manager_type> ModuleLoader::security_manager_creator = 0;
boost::function<api_creators_func::logger_type> ModuleLoader::logger_creator = 0;
boost::function<api_creators_func::storage_type> ModuleLoader::storage_creator = 0;

ModuleLoader::ModuleLoader() {
  if (security_manager_creator.empty()) {
    security_manager_creator = load_create_function<api_creators_func::security_manager_type>(
        "security_manager_", "create",
        [](App &app, ref_<LinkStrand> strand) {
          return make_ref_<SimpleSecurityManager>();
        });
  }

  if (logger_creator.empty()) {
    logger_creator = load_create_function<api_creators_func::logger_type>(
        "logger_", "create",
        [](App &app, ref_<LinkStrand> strand) {
          return std::unique_ptr<Logger>(new ConsoleLogger());
        });
  }

  if (storage_creator.empty()) {
    storage_creator = load_create_function<api_creators_func::storage_type>(
        "storage_", "create",
        [](App &app, ref_<LinkStrand> strand) {
          return std::unique_ptr<Storage>(new SimpleStorage(&app.io_service()));
        });
  }
}

std::unique_ptr<Logger> ModuleLoader::new_logger(App& app, ref_<LinkStrand> strand){
  return logger_creator(app, strand);
}

std::unique_ptr<Storage> ModuleLoader::new_storage(App& app, ref_<LinkStrand> strand){
  return storage_creator(app, strand);
}

ref_<SecurityManager> ModuleLoader::new_security_manager(App& app, ref_<LinkStrand> strand)
{
  return security_manager_creator(app, strand);
}

template<typename T>
boost::function<T> ModuleLoader::load_create_function(
    string_ module_name, string_ function_name,
    boost::function<T> default_function) {

  bf::path lib_path("./libs");
  auto module_suffix = boost::dll::shared_library::suffix();

  // There is no module directory returning default function
  if (!bf::is_directory(lib_path)) return default_function;

  // Iterate directory files
  std::vector<bf::path> module_paths;
  bf::directory_iterator end_itr;

  for (bf::directory_iterator itr(lib_path); itr != end_itr; ++itr) {
    if (bf::is_regular_file(itr->path())) {
      auto file_name = itr->path().filename().string();
      auto file_suffix = itr->path().extension();

      if (file_name.compare(0, module_name.length(), module_name) == 0 &&
          file_suffix == module_suffix) {
        module_paths.push_back(itr->path());
      }
    }
  }

  // analyze results
  bf::path shared_library_path;

  if (module_paths.size() == 1) shared_library_path = module_paths[0];

  if (module_paths.size() > 1) {
    std::cout << "There is more than one module for " << module_name
              << "so we can't decide which one will be used" << std::endl;
    exit(1);
  }
  if (shared_library_path.empty()) return default_function;

  // we found our shared lib, get create function from it
  try {
    return boost::dll::import_alias<T>(shared_library_path, function_name);
  } catch (int e) {
    std::cout << "Exception while loading " << module_name << std::endl;
  }

  // error occurred so returning default again
  return default_function;
}

}