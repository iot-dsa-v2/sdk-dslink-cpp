#include "gracefully_closable.h"

#include "app.h"

namespace dsa {
GracefullyClosable::GracefullyClosable(std::shared_ptr<App> app) : _app(app) {}

GracefullyClosable::~GracefullyClosable() {
//  _app.unregister_component(this);
}

void GracefullyClosable::register_this() {
  _app->register_component(shared_from_this());
}
}  // namespace dsa