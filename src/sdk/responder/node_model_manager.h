#ifndef DSA_SDK_NODE_MODEL_MANAGER_H_
#define DSA_SDK_NODE_MODEL_MANAGER_H_

#include <map>
#include <string>
#include <memory>

#include <boost/thread/shared_mutex.hpp>

namespace dsa {
class NodeModel;
class NodeState;

class NodeModelManager {
 private:
  std::map<std::string, std::shared_ptr<NodeModel>> _models;
  boost::shared_mutex _models_key;

 public:
  // should find and attach the node state to the model when it becomes available
  void find_model(std::shared_ptr<NodeState> node_state);

  // should immediately return the node model if it exists
  // return nullptr is model doesn't exist
  std::shared_ptr<NodeModel> get_model(const std::string &path);
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_MODEL_MANAGER_H_
