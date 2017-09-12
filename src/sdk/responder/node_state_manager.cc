#include "dsa_common.h"

#include "node_state_manager.h"

#include <boost/algorithm/string.hpp>

#include "core/session.h"
#include "node_model.h"

namespace dsa {

NodeStateManager::NodeStateManager(ref_<NodeModel> &&root_model)
    : _root(*this, std::move(root_model)) {}

void NodeStateManager::remove_state(const std::string &path) {
  _states.erase(path);
}

ref_<NodeState> NodeStateManager::get_state(const Path &path) {
  if (path.is_root()) {
    return _root.get_ref();
  }
  auto result = _states.find(path.full_str());
  if (result != _states.end()) {
    return result->second->get_ref();
  }

  // register it in global map for quick access
  ref_<NodeState> state = _root.get_child(path, true);
  if (!state->in_use()) {
    _states[path.full_str()] = state.get();
    state->_path = path;
    if (state->_model_status == NodeState::MODEL_UNKNOWN) {
      // TODO:state->set_model(_model_manager->get_model(path));
    }
  }
  return std::move(state);
}
ref_<NodeState> NodeStateManager::check_state(const Path &path) {
  if (path.is_root()) {
    return _root.get_ref();
  }
  auto result = _states.find(path.full_str());
  if (result != _states.end()) {
    return result->second->get_ref();
  }
  return _root.get_child(path, false);
}

void NodeStateManager::model_added(const Path &path, ref_<NodeModel> &model) {
  ref_<NodeState> state = check_state(path);
  if (state != nullptr) {
  }
}
void NodeStateManager::model_deleted(const Path &path) {}

void NodeStateManager::add(ref_<OutgoingSubscribeStream> &&stream) {
  ref_<NodeState> state = get_state(stream->path);
  state->subscribe(std::move(stream));
}
void NodeStateManager::add(ref_<OutgoingListStream> &&stream) {}
void NodeStateManager::add(ref_<OutgoingInvokeStream> &&stream) {}
void NodeStateManager::add(ref_<OutgoingSetStream> &&stream) {}

}  // namespace dsa