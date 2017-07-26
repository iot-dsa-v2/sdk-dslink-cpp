#ifndef DSA_SDK_NODE_MODEL_H_
#define DSA_SDK_NODE_MODEL_H_

#include <memory>
#include <vector>

#include "dsa/util.h"

namespace dsa {
class NodeState;

// interface of the real model logic
class NodeModel {
 private:
  std::shared_ptr<NodeState> _state;

 public:
  void update_value(Buffer::SharedBuffer buf);
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_MODEL_H_