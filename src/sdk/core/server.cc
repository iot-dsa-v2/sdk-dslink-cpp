#include "dsa_common.h"

#include "server.h"

#include "core/session_manager.h"

namespace dsa {

Server::Server(WrapperStrand & config)
    : _strand(config.strand) {}

void Server::destroy_impl() {
//  _strand->session_manager().destroy();
  _strand->destroy();
  _strand.reset();
}

}  // namespace dsa