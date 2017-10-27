#ifndef DSA_SDK_CONFIG_LOADER_H
#define DSA_SDK_CONFIG_LOADER_H

#include "core/config.h"

namespace dsa {
class ConfigLoader : public WrapperConfig {
 public:
  ConfigLoader(int argc, const char *argv[], const string_ &link_name,
               const string_ &version);

 private:
  void parse_url(const string_ &url);
  void parse_log(const string_ &log);
  void parse_name(const string_ &name);
};
}

#endif  // DSA_SDK_CONFIG_LOADER_H
