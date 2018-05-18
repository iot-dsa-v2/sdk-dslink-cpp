#include "dsa_common.h"

#include "v1_session_manager.h"

#include "responder/node_state_manager.h"

namespace dsa {
V1SessionManager::V1SessionManager(const LinkStrandRef &strand,
                                   NodeStateManager &state_manager)
    : _strand(strand), _state_manager(state_manager.get_ref()) {}
V1SessionManager::~V1SessionManager() = default;

void V1SessionManager::destroy_impl() { _state_manager.reset(); }
}  // namespace dsa