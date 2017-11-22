#ifndef DSA_BROKER_REMOTE_NODE_H
#define DSA_BROKER_REMOTE_NODE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/model_base.h"

namespace dsa {

class Session;

class RemoteNode : public NodeModelBase {
  const string_ _remote_path;
  ref_<Session> _remote_session;

 public:
  RemoteNode(LinkStrandRef &&strand, const string_ &remote_path,
             ref_<Session> &&session);
  ~RemoteNode() override;

  // when return true, destroy() will be called by NodeState
  // and model will be removed from the node tree
  bool periodic_check(size_t ts) override;

  bool allows_runtime_child_change() override { return true; }
  ModelRef on_demand_create_child(const Path &path) override;

  /// subscribe

  void on_subscribe(const SubscribeOptions &options,
                    bool first_request) override;
  void on_unsubscribe() override;

  /// list
 protected:
  void on_list(BaseOutgoingListStream &stream, bool first_request) override;
  void on_unlist() override;

  // get the summery Map when it's listed in a parent node
  BytesRef &get_summary() override;

  /// invoke
 public:
  void invoke(ref_<OutgoingInvokeStream> &&stream,
              ref_<NodeState> &parent) override;

  /// set
 public:
  void set(ref_<OutgoingSetStream> &&stream) override;
};
}
#endif  // DSA_BROKER_REMOTE_NODE_H
