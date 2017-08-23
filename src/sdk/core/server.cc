#include <module/default/simple_security_manager.h>

#include "app.h"

namespace dsa {

Server::Server(WrapperConfig & config)
    : _strand(config.strand) {}

void Server::close() {
  _strand->session_manager().end_all_sessions();
  _strand.reset();
  Closable::close();
}

}  // namespace dsa