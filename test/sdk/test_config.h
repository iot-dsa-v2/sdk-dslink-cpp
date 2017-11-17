#ifndef DSA_TEST_MODULES_H
#define DSA_TEST_MODULES_H

#include "core/editable_strand.h"

namespace dsa {

class App;
class Client;

class TestConfig : public WrapperStrand {
 private:
  static uint16_t _port;

 public:
  explicit TestConfig(App &app, bool async = false);

  WrapperStrand get_client_wrapper_strand(App &app, bool async = false);
};

void destroy_client_in_strand(ref_<Client> &client);
}
#endif  // PROJECT_TEST_MODULES_H
