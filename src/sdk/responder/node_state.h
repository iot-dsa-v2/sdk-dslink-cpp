#ifndef DSA_SDK_NODE_STATE_H_
#define DSA_SDK_NODE_STATE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <string>
#include <unordered_map>

#include "core/link_strand.h"

#include "message/response/subscribe_response_message.h"
#include "node_model.h"
#include "stream/responder/outgoing_subscribe_stream.h"

namespace dsa {
class OutgoingListStream;
class NodeModel;

class NodeStateOwner {
 public:
  virtual void remove_state(const std::string &path) = 0;
};

// maintain streams of a node
class NodeState : public EnableRef<NodeState> {
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
  std::unordered_map<std::string, ref_<NodeState>> _children;

  ref_<NodeModel> _model;
  ModelStatus _model_status = MODEL_UNKNOWN;

  // subscription related properties
  std::unordered_map<OutgoingSubscribeStream *, ref_<OutgoingSubscribeStream>>
      _subscription_streams;
  SubscribeOptions _merged_subscribe_options;
  void check_subscribe_options();

 public:
  explicit NodeState(NodeStateOwner &owner);
  virtual ~NodeState() = default;

  bool registered() { return _path.data() != nullptr; }

  ref_<NodeState> get_child(const std::string &name, bool create);
  ref_<NodeState> create_child(const Path &path, NodeState &last_modeled_state);
  ref_<NodeState> find_child(const Path &path);

  void remove_child(const std::string &name);

  void set_model(ref_<NodeModel> &&model);
  ref_<NodeModel> &get_model() { return _model; }

  //////////////////////////
  // Getters
  //////////////////////////
  const std::string &path() { return _path.full_str(); }

  /////////////////////////
  // Other
  /////////////////////////
  void new_subscribe_response(SubscribeResponseMessageCRef &&message);

  void subscribe(ref_<OutgoingSubscribeStream> &&stream);
  void list(ref_<OutgoingListStream> &&stream);
};

class NodeStateChild : public NodeState {
  ref_<NodeState> _parent;

 public:
  const std::string name;

  NodeStateChild(NodeStateOwner &owner, ref_<NodeState> parent,
                 const std::string &name);
};

class NodeStateRoot : public NodeState {
 public:
  explicit NodeStateRoot(NodeStateOwner &owner, ref_<NodeModel> &&model);
};

}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_H_
