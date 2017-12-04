#ifndef DSA_TEST_MODULES_H
#define DSA_TEST_MODULES_H

#include <dslink.h>
#include "core/editable_strand.h"

namespace dsa {

class App;
class Client;

class TestConfig : public WrapperStrand {
 private:
  std::shared_ptr<App> app;

 public:
  explicit TestConfig(std::shared_ptr<App>& app, bool async = false);

  WrapperStrand get_client_wrapper_strand();

  std::shared_ptr<TcpServer> create_server();
  ref_<DsLink> create_dslink(bool async = true);
};

void destroy_client_in_strand(ref_<Client> &client);
void destroy_dslink_in_strand(ref_<DsLink> &dslink);
}
#endif  // PROJECT_TEST_MODULES_H
