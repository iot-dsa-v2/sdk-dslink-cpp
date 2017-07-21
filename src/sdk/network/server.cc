#include "server.h"

#include "session.h"

namespace dsa {

Server::Server(const App &app) : _app(app) {}

void Server::stop() {
//  destroy();
}

}  // namespace dsa