#ifndef DSA_BROKER_MODULE_LOADER_H
#define DSA_BROKER_MODULE_LOADER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "util/enable_ref.h"

namespace dsa {

class BrokerConfig;

class ModuleLoader {
  public :
  ModuleLoader(ref_<BrokerConfig> );
};
}

#endif  // DSA_BROKER_MODULE_LOADER_H
