#ifndef DSA_SDK_NODE_MODEL_MANAGER_H_
#define DSA_SDK_NODE_MODEL_MANAGER_H_

#include <map>
#include <string>
#include <memory>

#include "util/enable_shared.h"

namespace dsa {
class NodeModel;
class NodeState;

class NodeModelManager {
 private:
  std::map<std::string, shared_ptr_<NodeModel>> _models;

 public:
  // should find and attach the node state to the model when it becomes available
  void find_model(intrusive_ptr_<NodeState> node_state);

  // should immediately return the node model if it exists
  // return nullptr is model doesn't exist
  shared_ptr_<NodeModel> get_model(const std::string &path);
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_MODEL_MANAGER_H_
