#include "dsa_common.h"

#include "node_state_manager.h"

namespace dsa {

NodeStateManager::NodeStateManager(LinkStrandPtr strand)
    : strand(std::move(strand)) {}

const intrusive_ptr_<NodeState> &NodeStateManager::get_or_create(std::string path) {
  if (_node_states.count(path) == 0) {
    _node_states[path] = make_intrusive_<NodeState>(strand, path);
  }
  return _node_states.at(path);
}

}  // namespace dsa