#ifndef  DSA_SDK_SERVER_H
#define  DSA_SDK_SERVER_H

#include "security_context.h"

namespace dsa {
class Server {
 private:
  SecurityContext &_security_context;

 public:
  Server(SecurityContext &security_context);
  const SecurityContext &security_context();
};
}  // namespace dsa

#endif  // DSA_SDK_SERVER_H
