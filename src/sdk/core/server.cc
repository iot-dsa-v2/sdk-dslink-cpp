#include "dsa_common.h"

#include "server.h"

#include "module/session_manager.h"

namespace dsa {

Server::Server(WrapperStrand & config)
    : _strand(config.strand) {}

void Server::destroy_impl() {
//  _strand->session_manager().destroy();
}

}  // namespace dsa