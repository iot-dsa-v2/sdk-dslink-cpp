#include "dsa_common.h"

#include "node_state.h"

#include "core/session.h"

#include "stream/responder/outgoing_list_stream.h"
#include "stream/responder/outgoing_subscribe_stream.h"

namespace dsa {

NodeState::NodeState(NodeStateOwner &owner) : _owner(owner) {}

ref_<NodeState> NodeState::get_child(const Path &path) {
  const std::string &name = path.current();

  // find existing node
  auto result = _children.find(path.full_str());
  if (result != _children.end()) {
    if (path.is_last()) {
      return result->second->get_ref();
    }
    return result->second->get_child(path.next());
  }

  // create new node
  _children[name] = new NodeStateChild(_owner, get_ref(), path.current());
  if (path.is_last()) {
    return _children[name]->get_ref();
  }
  return _children[name]->get_child(path.next());
}

void NodeState::remove_child(const std::string &path) { _children.erase(path); }

void NodeState::set_model(ModelRef model) { _model = std::move(model); }

void NodeState::new_message(ref_<SubscribeResponseMessage> &&message) {
  _last_value = message;
  for (auto &it : _subscription_streams) {
    auto &stream = dynamic_cast<ref_<OutgoingSubscribeStream> &>(*it);
    stream->send_message(ref_<SubscribeResponseMessage>(message));
  }
}

void NodeState::add_stream(ref_<OutgoingSubscribeStream> p) {
  _subscription_streams.insert(std::move(p));
}
void NodeState::add_stream(ref_<OutgoingListStream> p) {
  //_list_streams.insert(std::move(p));
}

void NodeState::remove_stream(ref_<OutgoingSubscribeStream> &p) {
  _subscription_streams.erase(std::move(p));
}
void NodeState::remove_stream(ref_<OutgoingListStream> &p) {
  //_list_streams.erase(std::move(p));
}

NodeStateChild::NodeStateChild(NodeStateOwner &owner, ref_<NodeState> parent,
                               const std::string &name)
    : NodeState(owner), _parent(std::move(parent)), name(name) {}
NodeStateChild::~NodeStateChild() {
  if (_path.data() != nullptr) {
    _owner.remove_node(_path.full_str());
  }
  _parent->remove_child(name);
  _parent.reset();
}

NodeStateRoot::NodeStateRoot(NodeStateOwner &owner) : NodeState(owner) {
  // keep a ref so it won't be deleted by smart pointer
  // the instance will always be non-pointer member in NodeStateManager
  intrusive_ptr_add_ref(this);
};

}  // namespace dsa
