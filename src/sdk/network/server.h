#ifndef  DSA_SDK_SERVER_H
#define  DSA_SDK_SERVER_H

#include "security_context.h"
#include "app.h"

namespace dsa {
class Server {
 protected:
  const App &_app;

 public:
  Server(const App &app);
};
}  // namespace dsa

#endif  // DSA_SDK_SERVER_H
