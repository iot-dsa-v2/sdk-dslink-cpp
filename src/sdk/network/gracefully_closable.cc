#include "dsa_common.h"

#include "gracefully_closable.h"

#include <utility>

#include "app.h"

namespace dsa {
GracefullyClosable::GracefullyClosable(const shared_ptr_<App> &app) : _app(app) {}

}  // namespace dsa