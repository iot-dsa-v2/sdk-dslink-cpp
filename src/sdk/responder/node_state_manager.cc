#include "dsa_common.h"

#include "node_state_manager.h"

#include "core/app.h"

namespace dsa {

NodeStateManager::NodeStateManager(const App &app)
    : _io_service(app.io_service()) {}

const shared_ptr_<NodeState> &NodeStateManager::get_or_create(
    std::string path) {
  if (_node_states.count(path) == 0) {
    _node_states[path] = make_shared_<NodeState>(_io_service, path);
  }
  return _node_states.at(path);
}

}  // namespace dsa