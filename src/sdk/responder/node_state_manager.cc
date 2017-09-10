#include "dsa_common.h"

#include "node_state_manager.h"

#include <boost/algorithm/string.hpp>

#include "core/session.h"

namespace dsa {

NodeStateManager::NodeStateManager() : _root(*this) {}

void NodeStateManager::remove_node(const std::string &path) {
  _states.erase(path);
}

ref_<NodeState> NodeStateManager::get_node(const Path &path) {
  if (path.is_root()) {
    return _root.get_ref();
  }
  auto result = _states.find(path.full_str());
  if (result != _states.end()) {
    return result->second->get_ref();
  }

  // register it in global map for quick access
  ref_<NodeState> state = _root.get_child(path);
  if (state->_path.data() == nullptr) {
    state->_path = path;
    _states[path.full_str()] = state.get();
  }
  return std::move(state);
}

void NodeStateManager::add(ref_<OutgoingSubscribeStream> &stream) {}
void NodeStateManager::add(ref_<OutgoingListStream> &stream) {}
void NodeStateManager::add(ref_<OutgoingInvokeStream> &stream) {}
void NodeStateManager::add(ref_<OutgoingSetStream> &stream) {}

}  // namespace dsa