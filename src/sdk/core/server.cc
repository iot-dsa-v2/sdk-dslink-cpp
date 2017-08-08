#include "dsa_common.h"

#include "app.h"
#include "server.h"
#include "session.h"
#include "session_manager.h"

namespace dsa {

Server::Server(const App &app, const Config &config)
    : _app(&app),
      config(config),
      _session_manager(new SessionManager(app.strand())) {}

void Server::on_session_connected(const shared_ptr_<Session> &session) {}

void Server::close() {
  if (_session_manager != nullptr) {
    _session_manager->end_all_sessions();
  }
}

}  // namespace dsa