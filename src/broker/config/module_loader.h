#ifndef DSA_BROKER_MODULE_LOADER_H
#define DSA_BROKER_MODULE_LOADER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <memory>

#include "util/enable_ref.h"

namespace dsa {
class Logger;
class BrokerConfig;
class SecurityManager;
class App;
class LinkStrand;

class ModuleLoader {
 public:
  explicit ModuleLoader(ref_<BrokerConfig>);
  std::unique_ptr<Logger> new_logger(App& app, ref_<LinkStrand> strand);
  ref_<SecurityManager> new_security_manager(App& app,
                                             ref_<LinkStrand> strand);
};
}

#endif  // DSA_BROKER_MODULE_LOADER_H
