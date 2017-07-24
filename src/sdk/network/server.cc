#include "server.h"

#include "session.h"
#include "session_manager.h"

namespace dsa {

Server::Server(App &app) : GracefullyClosable(app), _session_manager(new SessionManager) {}

void Server::stop() {
  if (_session_manager != nullptr) {
    _session_manager->end_all_sessions();
  }
}

}  // namespace dsa