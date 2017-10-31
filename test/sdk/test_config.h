#ifndef DSA_TEST_MODULES_H
#define DSA_TEST_MODULES_H

#include "core/config.h"

namespace dsa {

class App;
class Client;

class TestConfig : public WrapperConfig {
 private:
  static uint16_t _port;

 public:
  explicit TestConfig(App &app, bool async = false);

  WrapperConfig get_client_config(App &app, bool async = false);
};

void destroy_client_in_strand(ref_<Client> &client);
}
#endif  // PROJECT_TEST_MODULES_H
