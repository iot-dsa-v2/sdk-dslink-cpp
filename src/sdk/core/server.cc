#include "dsa_common.h"

#include "server.h"

#include "module/session_manager.h"

namespace dsa {

Server::Server(WrapperStrand& config)
    : _shared_strand(share_strand_(config.strand)),
      _dsid_prefix(config.dsid_prefix) {}

}  // namespace dsa