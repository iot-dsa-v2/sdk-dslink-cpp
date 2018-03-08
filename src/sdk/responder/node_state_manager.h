#ifndef DSA_SDK_NODE_STATE_MANAGER_H
#define DSA_SDK_NODE_STATE_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <string>
#include <unordered_map>

#include "core/link_strand.h"
#include "module/stream_acceptor.h"
#include "node_state.h"

namespace dsa {

class NodeStateManager final : public OutgoingStreamAcceptor,
                               public NodeStateOwner {
 private:
  ref_<NodeStateRoot> _root;
  std::unordered_map<string_, ref_<NodeState>> _states;

  // find or create a state
  ref_<NodeState> get_state(const Path &path);

  TimerRef _timer;
  // clear unused nodestates every minute
  bool _on_timer(bool canceled);
  // clear states if size is bigger than:
  size_t _check_states_size_threshold = 1000;
  size_t _timer_skipped = 0;

 protected:
  void destroy_impl() final;

 public:
  NodeStateManager(LinkStrand &strand, ModelRef &&root_model,
                   size_t timer_interval = 60);
  ~NodeStateManager() final;

  // find a state if it exists
  ref_<NodeState> check_state(const Path &path);

  void remove_state(const string_ &path) final;

  void model_added(const Path &path, ModelRef &model);
  void model_deleted(const Path &path);

  void add(ref_<OutgoingSubscribeStream> &&stream) final;
  void add(ref_<OutgoingListStream> &&stream) final;
  void add(ref_<OutgoingInvokeStream> &&stream) final;
  void add(ref_<OutgoingSetStream> &&stream) final;

  ref_<NodeModel> get_profile(const string_ &path, bool dsa_standard) final;
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_MANAGER_H
