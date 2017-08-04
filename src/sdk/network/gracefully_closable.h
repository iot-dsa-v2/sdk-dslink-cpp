#ifndef DSA_SDK_NETWORK_GRACEFULLY_CLOSABLE_H_
#define DSA_SDK_NETWORK_GRACEFULLY_CLOSABLE_H_

#include "util/enable_shared.h"

namespace dsa {
class App;

// interface that classes must adhere to in order to perform a graceful stop
class GracefullyClosable : public InheritableEnableShared<GracefullyClosable> {
  
 public:
  shared_ptr_<App> _app;

  // this should gracefully stop any running process
  // that the inheriting object has running
  virtual void stop() = 0;

  // this ensures that the component is registered with the app
  explicit GracefullyClosable(const shared_ptr_<App> &app);

};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_GRACEFULLY_CLOSABLE_H_
