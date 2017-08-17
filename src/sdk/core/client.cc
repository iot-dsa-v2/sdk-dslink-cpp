#include <module/default/simple_security_manager.h>
#include "dsa_common.h"

#include "client.h"
#include "network/client_connection.h"

namespace dsa {
Client::Client(const Config &config, intrusive_ptr_<Session> &&session)
    : _strand(config.strand),
      _security_manager(config.security_manager == nullptr ? make_intrusive_<SimpleSecurityManager>() : config.security_manager),
      _node_state_manager(config.state_manager == nullptr ? make_intrusive_<NodeStateManager>(config.strand) : config.state_manager),
      _node_model_manager(config.model_manager == nullptr ? make_intrusive_<NodeModelManager>() : config.model_manager),
      _client_token(config.client_token),
      _session(session == nullptr ? make_intrusive_<Session>(*this, "", nullptr) : std::move(session)) {}

void Client::close() {
  _connection->close();
}
}  // namespace dsa
