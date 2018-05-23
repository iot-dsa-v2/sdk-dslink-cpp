#include "dsa_common.h"

#include "v1_session_manager.h"

#include "responder/node_state_manager.h"

namespace dsa {
V1SessionManager::V1SessionManager(const LinkStrandRef& strand,
                                   NodeStateManager& state_manager)
    : _strand(strand),
      _state_manager(state_manager.get_ref()),
      _shared_ptr(SharedRef<V1SessionManager>::make(get_ref(), strand)) {}
V1SessionManager::~V1SessionManager() = default;

void V1SessionManager::destroy_impl() { _state_manager.reset(); }

void V1SessionManager::on_conn(const string_& dsid, const string_& token,
                               const string_& body,
                               std::function<void(const string_&)>&& callback) {
  callback("{}");
}
void V1SessionManager::on_ws(shared_ptr_<Websocket>&& ws) {}
}  // namespace dsa