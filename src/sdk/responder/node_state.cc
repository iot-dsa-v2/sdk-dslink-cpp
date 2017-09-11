#include "dsa_common.h"

#include "node_state.h"

#include "core/session.h"

#include "stream/responder/outgoing_list_stream.h"
#include "stream/responder/outgoing_subscribe_stream.h"

namespace dsa {

NodeState::NodeState(NodeStateOwner &owner) : _owner(owner) {}

ref_<NodeState> NodeState::get_child(const Path &path, bool create) {
  const std::string &name = path.current();

  // find existing node
  auto result = _children.find(path.full_str());
  if (result != _children.end()) {
    if (path.is_last()) {
      return result->second->get_ref();
    }
    return result->second->get_child(path.next(), create);
  } else if (create) {
    // create new node
    _children[name] = new NodeStateChild(_owner, get_ref(), path.current());
    if (path.is_last()) {
      return _children[name]->get_ref();
    }
    return _children[name]->get_child(path.next(), true);
  }
  // not found, return a nullptr
  return ref_<NodeState>();
}

void NodeState::remove_child(const std::string &name) { _children.erase(name); }

void NodeState::set_model(ref_<NodeModel> &model) {
  if (model == nullptr) {
    _model.reset();
    if (model == NodeModel::WAITING_REF) {
      _model_status = MODEL_WAITING;
    } else if (model == NodeModel::UNAVAILIBLE_REF) {
      _model_status = MODEL_UNAVAILABLE;
    } else {
      _model_status = MODEL_INVALID;
    }
  } else {
    _model = std::move(model);
    _model_status = MODEL_CONNECTED;
  }
}
void NodeState::delete_model() {}

void NodeState::new_subscribe_response(SubscribeResponseMessageCRef &&message) {
  _last_subscribe_response = message;
  for (auto &it : _subscription_streams) {
    it.first->send_message(SubscribeResponseMessageCRef(message));
  }
}

void NodeState::check_subscribe_options() {
  SubscribeOptions new_options;
  for (auto &stream : _subscription_streams) {
    new_options.mergeFrom(stream.first->options());
  }
  if (new_options != _merged_subscribe_options && _model != nullptr) {
    if (new_options.is_empty()) {
      _model->subscribe(new_options, nullptr);
    } else {
      _model->subscribe(new_options, [ this, keep_ref = get_ref() ](
                                         SubscribeResponseMessageCRef && msg) {
        new_subscribe_response(std::move(msg));
      });
    }
  }
}

void NodeState::subscribe(ref_<OutgoingSubscribeStream> &&stream) {
  _subscription_streams[stream.get()] = stream;
  if (_merged_subscribe_options.mergeFrom(stream->options())) {
    // TODO update model;
  }
  stream->on_option_change([ this, keep_ref = get_ref() ](
      OutgoingSubscribeStream & stream, const SubscribeOptions &old_options) {
    if (stream.is_closed()) {
      _subscription_streams.erase(&stream);
      if (_merged_subscribe_options.needUpdateOnRemoval(stream.options())) {
        check_subscribe_options();
      }
    } else {
      if (_merged_subscribe_options.needUpdateOnChange(old_options,
                                                       stream.options())) {
        check_subscribe_options();
      }
    }
  });
  if (_last_subscribe_response != nullptr) {
    stream->send_message(_last_subscribe_response->get_ref());
  }
}
void NodeState::list(ref_<OutgoingListStream> &&stream) {
  //_list_streams.insert(std::move(p));
}

NodeStateChild::NodeStateChild(NodeStateOwner &owner, ref_<NodeState> parent,
                               const std::string &name)
    : NodeState(owner), _parent(std::move(parent)), name(name) {}
NodeStateChild::~NodeStateChild() {
  if (_path.data() != nullptr) {
    _owner.remove_state(_path.full_str());
  }
  _parent->remove_child(name);
}

NodeStateRoot::NodeStateRoot(NodeStateOwner &owner) : NodeState(owner) {
  // keep a ref so it won't be deleted by smart pointer
  // the instance will always be non-pointer member in NodeStateManager
  intrusive_ptr_add_ref(this);
};

}  // namespace dsa
