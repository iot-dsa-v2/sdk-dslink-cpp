#include "dsa_common.h"

#include "server.h"

#include "module/session_manager.h"

namespace dsa {

Server::Server(WrapperStrand& config) : _strand(share_strand_(config.strand)) {}

void Server::destroy_impl() {
  // make sure strand ref is rest within strand
  _strand.reset();
}

}  // namespace dsa