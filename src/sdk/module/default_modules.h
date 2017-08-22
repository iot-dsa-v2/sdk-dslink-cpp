#ifndef DSA_SDK_MODULE_DEFAULT_MODULES_H_
#define DSA_SDK_MODULE_DEFAULT_MODULES_H_

#include "core/config.h"

namespace dsa {
class App;
class ECDH;

class DefaultModules :public LinkConfig{

 public:
  DefaultModules(App &app);
  DefaultModules(App &app, boost::asio::io_service::strand * security_strand);
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_DEFAULT_MODULES_H_
