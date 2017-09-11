#ifndef DSA_SDK_NODE_STATE_H_
#define DSA_SDK_NODE_STATE_H_

#include <string>
#include <unordered_map>

#include "core/link_strand.h"

#include "message/response/subscribe_response_message.h"
#include "node_model.h"
#include "stream/responder/outgoing_subscribe_stream.h"

namespace dsa {
class OutgoingListStream;
class NodeModel;

typedef ref_<NodeModel> ModelRef;

class NodeStateOwner {
 public:
  virtual void remove_state(const std::string &path) = 0;
};

// maintain streams of a node
class NodeState : public EnableRef<NodeState> {
  enum ModelStatus : uint8_t {
    MODEL_UNKNOWN,
    MODEL_UNAVAILABLE, // currently not available, but might be created later
    MODEL_WAITING,
    MODEL_CONNECTED,
    MODEL_INVALID
  };

  friend class NodeStateManager;

 protected:
  NodeStateOwner &_owner;
  Path _path;
  ref_<NodeState> _parent;
  std::unordered_map<std::string, NodeState *> _children;

  ModelRef _model;
  ModelStatus _model_status = MODEL_UNKNOWN;

  // subscription related properties
  std::unordered_map<OutgoingSubscribeStream *, ref_<OutgoingSubscribeStream>>
      _subscription_streams;
  ref_<SubscribeResponseMessage> _last_subscribe_response;
  SubscribeOptions _merged_subscribe_options;
  void check_subscribe_options();

 public:
  explicit NodeState(NodeStateOwner &owner);
  virtual ~NodeState() = default;

  bool in_use() { return _path.data() != nullptr; }

  ref_<NodeState> get_child(const Path &path, bool create);

  void remove_child(const std::string &name);

  void set_model(ref_<NodeModel> &model);
  void delete_model();
  //////////////////////////
  // Getters
  //////////////////////////
  const std::string &path() { return _path.full_str(); }
  bool has_model() { return _model != nullptr; }

  /////////////////////////
  // Other
  /////////////////////////
  void new_message(ref_<SubscribeResponseMessage> &&message);

  void subscribe(ref_<OutgoingSubscribeStream> &&stream);
  void list(ref_<OutgoingListStream> &&stream);
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
