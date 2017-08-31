#ifndef DSA_SDK_NODE_STATE_MANAGER_H_
#define DSA_SDK_NODE_STATE_MANAGER_H_

#include <map>
#include <string>
#include <utility>

#include "core/link_strand.h"

#include "node_state.h"

namespace dsa {

class NodeStateManager {
 private:
  LinkStrandRef strand;
  std::map< std::string, ref_<NodeState> > _node_states;

 public:
  explicit NodeStateManager(LinkStrandRef & strand);

  const ref_<NodeState> &get_or_create(std::string path);
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_MANAGER_H_
