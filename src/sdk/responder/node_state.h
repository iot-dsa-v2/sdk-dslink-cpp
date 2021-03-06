#ifndef DSA_SDK_NODE_STATE_H
#define DSA_SDK_NODE_STATE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <string>
#include <unordered_map>

#include "core/link_strand.h"

#include "message/response/subscribe_response_message.h"
#include "model_base.h"

namespace dsa {

class OutgoingInvokeStream;
class OutgoingSetStream;
class NodeModelBase;

class NodeStateOwner {
 public:
  virtual void remove_state(const string_ &path) = 0;
};

class NodeStateWaitingCache {
 public:
  std::vector<ref_<OutgoingInvokeStream>> invokes;
  std::vector<ref_<OutgoingSetStream>> sets;

  NodeStateWaitingCache();
  ~NodeStateWaitingCache();
};

// maintain streams of a node
class NodeState : public DestroyableRef<NodeState> {
  enum ModelStatus : uint8_t {
    MODEL_UNKNOWN,
    MODEL_CONNECTED,
    MODEL_WAITING,
    MODEL_UNAVAILABLE,  // model currently not available, but might be created
                        // later
    MODEL_INVALID
  };

  friend class NodeStateManager;

 protected:
  NodeStateOwner &_owner;
  Path _path;
  ref_<NodeState> _parent;
  std::unordered_map<string_, ref_<NodeState>> _children;

  ModelRef _model;
  ModelStatus _model_status = MODEL_UNKNOWN;

  std::unique_ptr<NodeStateWaitingCache> _waiting_cache;

  // subscription related properties
  std::unordered_map<BaseOutgoingSubscribeStream *,
                     ref_<BaseOutgoingSubscribeStream>>
      _subscription_streams;
  std::unordered_map<BaseOutgoingListStream *, ref_<BaseOutgoingListStream>>
      _list_streams;

  SubscribeOptions _merged_subscribe_options;
  void check_subscribe_options();
  // check unavailable children that need to get model from new parent
  void connect_unavailable_children(NodeState &modeled_state);

  void destroy_impl() override;

 public:
  NodeState(NodeStateOwner &owner, ref_<NodeState> &&parent);
  virtual ~NodeState();

  bool registered() { return _path.data() != nullptr; }

  ref_<NodeState> get_parent() { return _parent; }
  ref_<NodeState> get_child(const string_ &name, bool create);
  ref_<NodeState> create_child(const Path &path, NodeState &last_modeled_state,
                               bool allows_runtime_change);
  ref_<NodeState> find_child(const Path &path);

  // remove the current model as well as all children models
  // return true when the state is no longer needed
  bool remove_model();
  void remove_child(const string_ &name);

  void set_model(ModelRef &&model);
  ModelRef &get_model() { return _model; }
  template <class ModelClass>
  ModelClass *model_cast() {
    return dynamic_cast<ModelClass *>(_model.get());
  }

  //////////////////////////
  // Getters
  //////////////////////////
  const string_ &get_full_path() { return _path.full_str(); }
  const Path &get_path() { return _path; }

  inline bool is_idle() {
    return _subscription_streams.empty() && _list_streams.empty() &&
           _waiting_cache == nullptr && _children.empty();
  }

  // return true when the state is no longer needed
  bool periodic_check(int64_t ts);

  /////////////////////////
  // Other
  /////////////////////////
  void new_subscribe_response(SubscribeResponseMessageCRef &&message);

  void subscribe(ref_<BaseOutgoingSubscribeStream> &&stream);

  void update_list_value(const string_ &key, const VarBytesRef &value);
  void update_response_status(Status status = Status::OK);
  void update_list_refreshed();
  void update_list_removed(const string_ &key);
  void update_list_pub_path(const string_ &path);

  void list(ref_<BaseOutgoingListStream> &&stream);

  void invoke(ref_<OutgoingInvokeStream> &&stream);

  void set(ref_<OutgoingSetStream> &&stream);
};

class NodeStateRoot : public NodeState {
 public:
  explicit NodeStateRoot(NodeStateOwner &owner, ModelRef &&model);
};

}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_H
