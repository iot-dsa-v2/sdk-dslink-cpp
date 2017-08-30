#include "dsa_common.h"

#include "server.h"

#include "app.h"

namespace dsa {

Server::Server(WrapperConfig & config)
    : _strand(config.strand) {}

void Server::close_impl() {
  _strand->session_manager().close();
  _strand.reset();
}

}  // namespace dsa