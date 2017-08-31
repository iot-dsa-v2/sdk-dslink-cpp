#include "dsa_common.h"

#include "node_state_manager.h"

#include "core/session.h"

namespace dsa {

NodeStateManager::NodeStateManager(LinkStrandRef & strand)
    : strand(strand) {}

const ref_<NodeState> &NodeStateManager::get_or_create(std::string path) {
  if (_node_states.count(path) == 0) {
    _node_states[path] = make_ref_<NodeState>(strand, path);
  }
  return _node_states.at(path);
}

}  // namespace dsa