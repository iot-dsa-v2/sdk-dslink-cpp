#include "dsa_common.h"

#include "v1_session_manager.h"

#include "responder/node_state_manager.h"

namespace dsa {
V1SessionManager::V1SessionManager(const LinkStrandRef& strand,
                                   NodeStateManager& state_manager)
    : _strand_state_manager(
          SharedRef<NodeStateManager>::make(state_manager.get_ref(), strand)) {}
V1SessionManager::~V1SessionManager() = default;

void V1SessionManager::post_in_strand(std::function<void()>&& callback) {
  _strand_state_manager->post(std::move(callback));
}
void V1SessionManager::destroy_impl() {}

string_ V1SessionManager::on_conn(const string_& dsid, const string_& token,
                                  const string_& body) {
  return "{}";
}
void V1SessionManager::on_ws(std::unique_ptr<Websocket>&& ws) {}
}  // namespace dsa