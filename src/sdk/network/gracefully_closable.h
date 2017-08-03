#ifndef DSA_SDK_NETWORK_GRACEFULLY_CLOSABLE_H_
#define DSA_SDK_NETWORK_GRACEFULLY_CLOSABLE_H_

#include "util/enable_shared.h"

namespace dsa {
class App;

// interface that classes must adhere to in order to perform a graceful stop
class GracefullyClosable : public InheritableEnableShared<GracefullyClosable> {
 private:
  size_t _id;
  
 public:
  shared_ptr_<App> _app;

  // this should gracefully stop any running process
  // that the inheriting object has running
  virtual void stop() = 0;

  // this ensures that the component is registered with the app
  explicit GracefullyClosable(const shared_ptr_<App> &app);

  // ensures that components remove themselves from register once dead
  ~GracefullyClosable() override;

  // must be called at some point after constructor
  void register_this();
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_GRACEFULLY_CLOSABLE_H_
