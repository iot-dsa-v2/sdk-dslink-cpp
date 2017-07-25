#include "node_state_manager.h"

namespace dsa {

std::shared_ptr<NodeState> NodeStateManager::get_or_create(std::string path) {
  boost::upgrade_lock<boost::shared_mutex> lock(_key);
  if (_node_states.count(path) < 1) {
    boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);
    _node_states[path] = std::make_shared<NodeState>(_app->io_service(), path);
  }
  return _node_states.at(path);
}

}  // namespace dsa