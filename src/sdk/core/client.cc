#include "dsa_common.h"

#include "client.h"

namespace dsa {
Client::Client(const App &app, const Config &config)
    : _app(&app), config(config) {}
}  // namespace dsa
