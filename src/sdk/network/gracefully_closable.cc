#include "dsa_common.h"

#include "gracefully_closable.h"

#include <utility>

#include "app.h"

namespace dsa {
GracefullyClosable::GracefullyClosable(const shared_ptr_<App> &app) : _app(app) {}

GracefullyClosable::~GracefullyClosable() {
  _app->unregister_component(_id);
}

void GracefullyClosable::register_this() {
  _id = _app->register_component(shared_from_this());
}
}  // namespace dsa