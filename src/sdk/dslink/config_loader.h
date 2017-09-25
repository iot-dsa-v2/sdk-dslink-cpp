#ifndef DSA_SDK_CONFIG_LOADER_H
#define DSA_SDK_CONFIG_LOADER_H

#include "core/config.h"

namespace dsa {
class ConfigLoader : public WrapperConfig{
 public:
  ConfigLoader(int argc, const char *argv[]);
};
}

#endif  // DSA_SDK_CONFIG_LOADER_H
