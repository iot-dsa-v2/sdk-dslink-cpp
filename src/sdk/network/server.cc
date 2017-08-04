#include "dsa_common.h"

#include "app.h"
#include "server.h"
#include "session.h"
#include "session_manager.h"

namespace dsa {

Server::Server(const App &app)
    : _app(&app),_session_manager(new SessionManager(app.strand())) {}

void Server::close() {
  if (_session_manager != nullptr) {
    _session_manager->end_all_sessions();
  }
}

}  // namespace dsa