#ifndef DSA_SDK_NODE_STATE_MANAGER_H_
#define DSA_SDK_NODE_STATE_MANAGER_H_

#include "stream/outgoing_stream_accepter.h"

#include <map>
#include <string>
#include <utility>
#include <unordered_map>

#include "core/link_strand.h"

#include "node_state.h"

namespace dsa {

class NodeStateManager : public OutgoingStreamAcceptor {
 private:

  ref_<NodeState> _root;
  std::unordered_map<std::string, ref_<NodeState>> _states;

  ref_<NodeState> &get_node(const Path &path);

 public:
  explicit NodeStateManager();

  void add(ref_<OutgoingSubscribeStream> &stream) override;
  void add(ref_<OutgoingListStream> &stream) override;
  void add(ref_<OutgoingInvokeStream> &stream) override;
  void add(ref_<OutgoingSetStream> &stream) override;
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_MANAGER_H_
