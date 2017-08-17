#ifndef DSA_SDK_MODULE_DEFAULT_MODULES_H_
#define DSA_SDK_MODULE_DEFAULT_MODULES_H_

#include <boost/asio.hpp>

#include "core/config.h"
#include "security_manager.h"

namespace dsa {
class App;
class ECDH;

class DefaultModules {
 protected:
  intrusive_ptr_<ECDH> ecdh;
  intrusive_ptr_<SecurityManager> security_manager;
  boost::asio::io_service::strand &strand;

 public:
  DefaultModules(App &app);
  DefaultModules(App &app, boost::asio::io_service::strand &security_strand);
  Config get_config();
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_DEFAULT_MODULES_H_
