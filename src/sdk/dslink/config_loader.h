#ifndef DSA_SDK_CONFIG_LOADER_H
#define DSA_SDK_CONFIG_LOADER_H

#include "core/config.h"

namespace dsa {
class App;

class ConfigLoader : public WrapperConfig {
 public:
  ConfigLoader(int argc, const char *argv[], const string_ &link_name,
               const string_ &version);

  App &get_app() const { return *_app; }

 private:
  std::unique_ptr<App> _app;

  std::unique_ptr<ECDH> load_private_key();

  void parse_thread(size_t thread);
  void parse_url(const string_ &url);
  void parse_log(const string_ &log, LinkConfig &config);
  void parse_name(const string_ &name);
};
}

#endif  // DSA_SDK_CONFIG_LOADER_H
