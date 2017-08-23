#include "dsa_common.h"

#include "node_model_manager.h"

namespace dsa {

shared_ptr_<NodeModel> NodeModelManager::get_model(const std::string &path) {
  if (_models.count(path) > 0)
    return _models.at(path);
  return nullptr;
}

void NodeModelManager::find_model(intrusive_ptr_<NodeState> node_state) {
  // TODO: implement default version for this
}

}  // namespace dsa