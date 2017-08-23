#include <module/default/simple_security_manager.h>
#include "dsa_common.h"

#include "app.h"
#include "server.h"
#include "session.h"
#include "session_manager.h"

namespace dsa {

Server::Server(WrapperConfig & config)
    : _strand(config.strand) {}

void Server::close() {
  _strand->session_manager().end_all_sessions();
  _strand.reset();
  Closable::close();
}

}  // namespace dsa