#include <module/default/simple_security_manager.h>
#include "dsa_common.h"

#include "app.h"
#include "server.h"
#include "session.h"
#include "session_manager.h"

namespace dsa {

Server::Server(const Config &config)
    : _security_manager(
    config.security_manager == nullptr ? make_intrusive_<SimpleSecurityManager>() : config.security_manager),
      _node_state_manager(
          config.state_manager == nullptr ? make_intrusive_<NodeStateManager>(config.strand) : config.state_manager),
      _node_model_manager(config.model_manager == nullptr ? make_intrusive_<NodeModelManager>() : config.model_manager),
      _strand(config.strand),
      _dsid_prefix(config.dsid_prefix),
      _ecdh(config.ecdh),
      _session_manager(*this, config) {}

void Server::on_session_connected(const shared_ptr_<Session> &session) {}

void Server::close() {
  _session_manager.end_all_sessions();
}

}  // namespace dsa