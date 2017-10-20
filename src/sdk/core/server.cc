#include "dsa_common.h"

#include "server.h"

#include "core/session_manager.h"

namespace dsa {

Server::Server(WrapperConfig & config)
    : _strand(config.strand) {}

void Server::destroy_impl() {
  _strand->session_manager().close();
  _strand.reset();
}

}  // namespace dsa