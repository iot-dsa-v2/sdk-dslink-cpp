#include "dsa_common.h"

#include "node_state_manager.h"

#include <boost/algorithm/string.hpp>

#include "core/session.h"

namespace dsa {

NodeStateManager::NodeStateManager() {}

ref_<NodeState> &NodeStateManager::get_node(const Path &path) {
  if (path.is_root()) {
    return _root;
  }
  auto result = _states.find(path.data->str);
  if (result != _states.end()) {
    return result->second;
  }

  // register it in global map for quick access
  auto &state = _root->get_child(path);
  if (!state->_registered) {
    state->_registered = true;
    _states[path.data->str] = state;
  }
  return state;
}

void NodeStateManager::add(ref_<OutgoingSubscribeStream> &stream) {}
void NodeStateManager::add(ref_<OutgoingListStream> &stream) {}
void NodeStateManager::add(ref_<OutgoingInvokeStream> &stream) {}
void NodeStateManager::add(ref_<OutgoingSetStream> &stream) {}

}  // namespace dsa