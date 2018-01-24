#ifndef DSA_BROKER_REMOTE_NODE_H
#define DSA_BROKER_REMOTE_NODE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>

#include "invoke_proxy.h"
#include "responder/model_base.h"
#include "set_proxy.h"

namespace dsa {

class Session;
class Connection;
class IncomingSubscribeStream;
class IncomingListStream;

class RemoteNode : public NodeModelBase {
  friend class RemoteInvokeProxy;
  friend class RemoteSetProxy;

 public:
  RemoteNode(LinkStrandRef &&strand, const string_ &remote_path,
             ref_<Session> &&session);
  ~RemoteNode() override;

  // when return true, destroy() will be called by NodeState
  // and model will be removed from the node tree
  bool periodic_check(const NodeState *state, int64_t ts) override {
    return true;
  }

  bool allows_runtime_child_change() override { return true; }
  ModelRef on_demand_create_child(const Path &path) override;

 protected:
  const string_ _remote_path;
  ref_<Session> _remote_session;
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
  MessageStatus _list_status_cache = MessageStatus::INITIALIZING;
  string_ _list_pub_path_cache;
  std::unordered_map<string_, VarBytesRef> _list_cache;
  ref_<IncomingListStream> _remote_list_stream;

  void on_list(BaseOutgoingListStream &stream, bool first_request) override;
  void on_unlist() override;

  /// invoke
 protected:
  std::unordered_map<RemoteInvokeProxy *, ref_<RemoteInvokeProxy>>
      _invoke_streams;
  void remove_invoke(RemoteInvokeProxy *invoke_proxy);

 public:
  void invoke(ref_<OutgoingInvokeStream> &&stream,
              ref_<NodeState> &parent) override;

  /// set
 protected:
  std::unordered_map<RemoteSetProxy *, ref_<RemoteSetProxy>> _set_streams;
  void remove_set(RemoteSetProxy *set_proxy);

 public:
  void set(ref_<OutgoingSetStream> &&stream) override;
};
}
#endif  // DSA_BROKER_REMOTE_NODE_H
