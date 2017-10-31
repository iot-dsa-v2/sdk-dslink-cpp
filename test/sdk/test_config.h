#ifndef DSA_TEST_MODULES_H
#define DSA_TEST_MODULES_H

#include "core/config.h"

namespace dsa {

class App;

class TestConfig : public WrapperConfig {
private:
  static uint16_t _port;
 public:
  explicit TestConfig(App &app, bool async = false);

  WrapperConfig get_client_config(App &app, bool async = false);

};

}
#endif  // PROJECT_TEST_MODULES_H
