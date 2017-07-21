#ifndef DSA_SDK_NETWORK_GRACEFULLY_CLOSABLE_H_
#define DSA_SDK_NETWORK_GRACEFULLY_CLOSABLE_H_

#include "util/enable_shared.h"

namespace dsa {
class App;

class GracefullyClosable : public InheritableEnableShared<GracefullyClosable> {
  // interface that classes must adhere to in order to perform a graceful stop
 public:
  App &_app;

  // this should gracefully stop any running process
  // that the inheriting object has running
  virtual void operator()() = 0;

  // this ensures that the component is registered with the app
  explicit GracefullyClosable(App &app);

  // ensures that components remove themselves from register once dead
  ~GracefullyClosable() override;

  // must be called at some point after constructor
  void register_this();
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_GRACEFULLY_CLOSABLE_H_
