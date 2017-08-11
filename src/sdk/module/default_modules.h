#ifndef DSA_SDK_MODULE_DEFAULT_MODULES_H_
#define DSA_SDK_MODULE_DEFAULT_MODULES_H_

#include "core/config.h"
#include "security_manager.h"
#include <boost/asio.hpp>

namespace dsa {
class ECDH;


class DefaultModules {
 protected:
  std::unique_ptr<ECDH> ecdh;
  std::unique_ptr<SecurityManager> security_manager;

 public:
  DefaultModules();
  DefaultModules(boost::asio::io_service::strand& strand);
  Config get_config();
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_DEFAULT_MODULES_H_
