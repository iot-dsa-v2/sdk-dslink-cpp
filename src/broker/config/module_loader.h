#ifndef DSA_BROKER_MODULE_LOADER_H
#define DSA_BROKER_MODULE_LOADER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <memory>

#include "boost/function.hpp"
#include "util/enable_ref.h"

namespace dsa {
class Logger;
class BrokerConfig;
class SecurityManager;
class App;
class LinkStrand;

// API function creators
namespace api_creators_func {
typedef ref_<SecurityManager>(security_manager_type)(App&, ref_<LinkStrand>);
typedef std::unique_ptr<Logger>(logger_type)(App&, ref_<LinkStrand>);
}

class ModuleLoader {
 private:
#ifndef __CYGWIN__
  static boost::function<api_creators_func::security_manager_type>
      security_manager_creator;
  static boost::function<api_creators_func::logger_type> logger_creator;

  template <typename T>
  boost::function<T> get_create_function(string_ module_name,
                                         string_ function_name,
                                         boost::function<T> default_function);
#endif

 public:
  explicit ModuleLoader(ref_<BrokerConfig>);
  std::unique_ptr<Logger> new_logger(App& app, ref_<LinkStrand> strand);
  ref_<SecurityManager> new_security_manager(App& app, ref_<LinkStrand> strand);
};
}

#endif  // DSA_BROKER_MODULE_LOADER_H
