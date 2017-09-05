#ifndef DSA_SDK_NODE_MODEL_MANAGER_H_
#define DSA_SDK_NODE_MODEL_MANAGER_H_

#include <map>
#include <string>
#include <memory>

#include "util/enable_intrusive.h"
#include "node_state.h"

namespace dsa {
class NodeModel;
class NodeState;

class NodeModelManager {
 private:
  std::map<std::string, ref_<NodeModel>> _models;

 public:
  // should find and attach the node state to the model when it becomes available
  void find_model(ref_<NodeState> node_state);

  // should immediately return the node model if it exists
  // return nullptr is model doesn't exist
  ref_<NodeModel> & get_model(const std::string &path);
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_MODEL_MANAGER_H_
