#include "dsa_common.h"

#include "node_state_manager.h"

#include <boost/algorithm/string.hpp>

#include "core/session.h"

namespace dsa {

NodeStateManager::NodeStateManager(LinkStrandRef &strand) : strand(strand) {}

ref_<NodeState> &NodeStateManager::get_node(const Path &path) {
  if (path.is_root()) {
    return _root;
  }
  return _root->get_child(path);
}

void NodeStateManager::add(ref_<OutgoingSubscribeStream> &stream) {}
void NodeStateManager::add(ref_<OutgoingListStream> &stream) {}
void NodeStateManager::add(ref_<OutgoingInvokeStream> &stream) {}
void NodeStateManager::add(ref_<OutgoingSetStream> &stream) {}

}  // namespace dsa