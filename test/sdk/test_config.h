#ifndef DSA_TEST_MODULES_H
#define DSA_TEST_MODULES_H

#include "core/config.h"

namespace dsa {

class TestConfig : public WrapperConfig {
private:
  static uint16_t _port;
 public:
  explicit TestConfig(App &app, bool async = false);

  WrapperConfig get_client_config(App &app, bool async = false);

  LinkConfig * get_link_config();
};

}
#endif  // PROJECT_TEST_MODULES_H
