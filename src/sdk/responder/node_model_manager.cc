#include "dsa_common.h"

#include "node_model_manager.h"
#include "node_model.h"

namespace dsa {

shared_ptr_<NodeModel> NodeModelManager::get_model(const std::string &path) {
  if (_models.count(path) > 0)
    return _models.at(path);
  return nullptr;
}

}  // namespace dsa