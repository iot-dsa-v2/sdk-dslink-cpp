#ifndef DSA_SDK_NODE_STATE_MANAGER_H_
#define DSA_SDK_NODE_STATE_MANAGER_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <string>
#include <unordered_map>

#include <boost/asio/deadline_timer.hpp>

#include "core/link_strand.h"
#include "node_state.h"
#include "stream/outgoing_stream_accepter.h"

namespace dsa {

class NodeStateManager : public OutgoingStreamAcceptor, public NodeStateOwner {
 private:
  ref_<NodeStateRoot> _root;
  std::unordered_map<string_, ref_<NodeState>> _states;

  ref_<NodeState> get_state(const Path &path);
  ref_<NodeState> check_state(const Path &path);

  boost::asio::deadline_timer _timer;

 public:
  explicit NodeStateManager(LinkStrand &strand, ModelRef &&root_model,
                            size_t timer_interval = 60);

  void remove_state(const string_ &path) override;

  void model_added(const Path &path, ModelRef &model);
  void model_deleted(const Path &path);

  void add(ref_<OutgoingSubscribeStream> &&stream) override;
  void add(ref_<OutgoingListStream> &&stream) override;
  void add(ref_<OutgoingInvokeStream> &&stream) override;
  void add(ref_<OutgoingSetStream> &&stream) override;
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_MANAGER_H_
