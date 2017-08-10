#ifndef DSA_SDK_MODULE_DEFAULT_MODULES_H_
#define DSA_SDK_MODULE_DEFAULT_MODULES_H_

#include "core/config.h"

#include "default/simple_security_manager.h"

#include "crypto/ecdh.h"

#include "core/config.h"

namespace dsa {
class DefaultModules {
 protected:
  ECDH ecdh;
  SimpleSecurityManager security_manager;

 public:
  DefaultModules();
  Config get_config();
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_DEFAULT_MODULES_H_
