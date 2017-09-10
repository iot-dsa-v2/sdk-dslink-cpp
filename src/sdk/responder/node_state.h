#ifndef DSA_SDK_NODE_STATE_H_
#define DSA_SDK_NODE_STATE_H_

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "core/link_strand.h"

#include "message/response/subscribe_response_message.h"
#include "node_model.h"
#include "stream/responder/outgoing_subscribe_stream.h"

namespace dsa {
class OutgoingListStream;
class NodeModel;

typedef ref_<NodeModel> ModelRef;

class MessageStreamHashFunc {
 public:
  size_t operator()(const ref_<MessageStream> &key) const {
    return reinterpret_cast<size_t>(key.get());
  }
};

class MessageStreamKeyCmp {
 public:
  bool operator()(const ref_<MessageStream> &t1,
                  const ref_<MessageStream> &t2) const {
    return t1.get() == t2.get();
  }
};

class NodeStateOwner {
 public:
  virtual void remove_node(const std::string &path) = 0;
};

// maintain streams of a node
class NodeState : public EnableRef<NodeState> {
  friend class NodeStateManager;

 protected:
  NodeStateOwner &_owner;
  Path _path;
  ref_<NodeState> _parent;
  std::unordered_map<std::string, NodeState *> _children;

  ModelRef _model;
  std::unordered_set<ref_<OutgoingSubscribeStream>, MessageStreamHashFunc,
                     MessageStreamKeyCmp>
      _subscription_streams;
  //  std::unordered_set<ref_<OutgoingListStream>, MessageStreamHashFunc,
  //                     MessageStreamKeyCmp>
  //      _list_streams;
  ref_<SubscribeResponseMessage> _last_value;

 public:
  explicit NodeState(NodeStateOwner &owner);
  virtual ~NodeState() = default;

  ref_<NodeState> get_child(const Path &path);
  void remove_child(const std::string &path);

  //////////////////////////
  // Getters
  //////////////////////////
  const std::string &path() { return _path.full_str(); }
  bool has_model() { return _model != nullptr; }

  //////////////////////////
  // Setters
  //////////////////////////
  void set_model(ModelRef model);

  /////////////////////////
  // Other
  /////////////////////////
  void new_message(ref_<SubscribeResponseMessage> &&message);

  void add_stream(ref_<OutgoingSubscribeStream> p);
  void add_stream(ref_<OutgoingListStream> p);

  void remove_stream(ref_<OutgoingSubscribeStream> &p);
  void remove_stream(ref_<OutgoingListStream> &p);
};

class NodeStateChild : public NodeState {
  ref_<NodeState> _parent;

 public:
  const std::string name;

  NodeStateChild(NodeStateOwner &owner, ref_<NodeState> parent,
                 const std::string &name);
  ~NodeStateChild() override;
};

class NodeStateRoot : public NodeState {
 public:
  explicit NodeStateRoot(NodeStateOwner &owner);
};

}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_H_
