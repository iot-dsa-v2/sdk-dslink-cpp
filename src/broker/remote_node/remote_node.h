#ifndef DSA_BROKER_REMOTE_NODE_H
#define DSA_BROKER_REMOTE_NODE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>
#include "responder/model_base.h"

namespace dsa {

class Session;
class Connection;
class IncomingSubscribeStream;
class IncomingListStream;

class RemoteNode : public NodeModelBase {
  const string_ _remote_path;
  ref_<Session> _remote_session;

 public:
  RemoteNode(LinkStrandRef &&strand, const string_ &remote_path,
             ref_<Session> &&session);
  ~RemoteNode() override;

  // when return true, destroy() will be called by NodeState
  // and model will be removed from the node tree
  bool periodic_check(size_t ts) override { return true; }

  bool allows_runtime_child_change() override { return true; }
  ModelRef on_demand_create_child(const Path &path) override;

protected:
  void destroy_impl() override;

  /// subscribe
 protected:
  ref_<IncomingSubscribeStream> _remote_subscribe_stream;

 public:
  void on_subscribe(const SubscribeOptions &options,
                    bool first_request) override;
  void on_unsubscribe() override;

  /// list
 protected:
  std::unordered_map<string_, BytesRef> _list_cache;
  ref_<IncomingListStream> _remote_list_stream;

  void on_list(BaseOutgoingListStream &stream, bool first_request) override;
  void on_unlist() override;

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
