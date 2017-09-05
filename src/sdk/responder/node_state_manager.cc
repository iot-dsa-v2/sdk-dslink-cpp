#include "dsa_common.h"

#include "node_state_manager.h"

#include <boost/algorithm/string.hpp>

#include "core/session.h"

namespace dsa {

NodeStateManager::NodeStateManager(LinkStrandRef &strand) : strand(strand) {}

ref_<NodeState> &NodeStateManager::get_or_create(const std::string &path) {

  std::vector< std::string > path_nodes;
  boost::split( path_nodes, path, boost::is_space()); // SplitVec == { "hello abc","ABC","aBc goodbye" }

  if (_node_states.count(path) == 0) {
    _node_states[path] = make_ref_<NodeState>(strand, path);
  }
  return _node_states.at(path);
}

void NodeStateManager::add(ref_<OutgoingSubscribeStream> &stream) {}
void NodeStateManager::add(ref_<OutgoingListStream> &stream) {}
void NodeStateManager::add(ref_<OutgoingInvokeStream> &stream) {}
void NodeStateManager::add(ref_<OutgoingSetStream> &stream) {}

}  // namespace dsa