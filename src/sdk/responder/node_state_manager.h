#ifndef DSA_SDK_NODE_STATE_MANAGER_H_
#define DSA_SDK_NODE_STATE_MANAGER_H_

#include "stream/outgoing_stream_accepter.h"

#include <map>
#include <string>
#include <unordered_map>
#include <utility>

#include "core/link_strand.h"

#include "node_state.h"

namespace dsa {

class NodeModelManager;

class NodeStateManager : public OutgoingStreamAcceptor, public NodeStateOwner {
 private:
  NodeStateRoot _root;
  std::unordered_map<std::string, NodeState *> _states;

  std::unique_ptr<NodeModelManager> _model_manager;

  ref_<NodeState> get_state(const Path &path);
  ref_<NodeState> check_state(const Path &path);

 public:
  explicit NodeStateManager();

  void remove_state(const std::string &path) override;

  void model_added(const Path &path, ref_<NodeModel> &model);
  void model_deleted(const Path &path);

  void add(ref_<OutgoingSubscribeStream> &&stream) override;
  void add(ref_<OutgoingListStream> &&stream) override;
  void add(ref_<OutgoingInvokeStream> &&stream) override;
  void add(ref_<OutgoingSetStream> &&stream) override;
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_MANAGER_H_
