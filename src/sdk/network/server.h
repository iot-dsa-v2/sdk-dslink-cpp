#ifndef  DSA_SDK_SERVER_H
#define  DSA_SDK_SERVER_H

#include "security_context.h"
#include "app.h"
#include "util/enable_shared.h"

namespace dsa {
class Server : public InheritableEnableShared<Server> {
 protected:
  const App &_app;

 public:
  explicit Server(const App &app);
  virtual void start() = 0;
  void stop();
};
}  // namespace dsa

#endif  // DSA_SDK_SERVER_H
