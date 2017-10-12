#include "dsa_common.h"

#include "node_state.h"

#include "core/session.h"

#include "stream/responder/outgoing_list_stream.h"
#include "stream/responder/outgoing_subscribe_stream.h"

namespace dsa {

NodeState::NodeState(NodeStateOwner &owner, ref_<NodeState> &&parent)
    : _owner(owner), _parent(std::move(parent)) {}
NodeState::~NodeState() = default;

ref_<NodeState> NodeState::get_child(const std::string &name, bool create) {
  // find existing node
  auto result = _children.find(name);
  if (result != _children.end()) {
    return result->second;
  } else if (create) {
    auto child = new NodeStateChild(_owner, get_ref(), name);
    child->_path = _path.get_child_path(name);
    _children[name] = child;
    return child->get_ref();
  }
  // not found, return a nullptr
  return ref_<NodeState>();
}
ref_<NodeState> NodeState::create_child(const Path &path,
                                        NodeState &last_modeled_state,
                                        bool allows_runtime_change) {
  const std::string &name = path.current_name();

  // find existing node
  auto result = _children.find(name);
  if (result != _children.end()) {
    if (path.is_last()) {
      return result->second;
    }
    // let child state decide
    if (result->second->_model != nullptr) {
      if (!allows_runtime_change &&
          result->second->_model->allows_runtime_child_change()) {
        allows_runtime_change = true;
      }
      return result->second->create_child(path.next(), *result->second,
                                          allows_runtime_change);
    } else {
      return result->second->create_child(path.next(), last_modeled_state,
                                          allows_runtime_change);
    }
  }
  if (allows_runtime_change) {
    // create new node
    ref_<NodeState> new_state = new NodeStateChild(_owner, get_ref(), name);
    _children[name] = new_state;
    if (path.is_last()) {
      new_state->_path = path;
      if (_model_status == MODEL_UNKNOWN) {
        set_model(last_modeled_state._model->on_demand_create_child(
            path.move_pos(last_modeled_state._path.current_pos() + 1)));
      } else if (_model_status == MODEL_CONNECTED) {
        set_model(_model->on_demand_create_child(path));
      } else {
        new_state->_model_status = _model_status;
      }
      return new_state;
    }
    return _children[name]->create_child(path.next(), last_modeled_state, true);
  }
  // not found, return a nullptr
  return ref_<NodeState>();
}

ref_<NodeState> NodeState::find_child(const Path &path) {
  const std::string &name = path.current_name();

  auto result = _children.find(name);
  if (result != _children.end()) {
    if (path.is_last()) {
      return result->second;
    }

    return result->second->find_child(path.next());
  }
  return ref_<NodeState>();
}

void NodeState::remove_child(const std::string &name) { _children.erase(name); }

void NodeState::set_model(ref_<NodeModelBase> &&model) {
  if (model == nullptr) {
    _model.reset();
    _model_status = MODEL_INVALID;
  } else if (model->_strand == nullptr){ // Other Invalid Models
    _model.reset();
    if (model == NodeModelBase::WAITING) {
      _model_status = MODEL_WAITING;
    } else if (model == NodeModelBase::UNAVAILABLE) {
      _model_status = MODEL_UNAVAILABLE;
    } else {
      _model_status = MODEL_INVALID;
    }
  }else {
    _model = std::move(model);
    _model->_state = get_ref();
    _model_status = MODEL_CONNECTED;
    _model->initialize();
  }
  // TODO send request to model
}
bool NodeState::periodic_check(size_t ts) {
  for (auto it = _children.begin(); it != _children.end();) {
    if (it->second->periodic_check(ts)) {
      it = _children.erase(it);
    } else {
      it++;
    }
  }

  if ((_model == nullptr ||
       _model->periodic_check(ts))  // check if model is still in use
      && _children.empty() && _subscription_streams.empty()) {
    close_impl();
    return true;
  }
  return false;
}

void NodeState::new_subscribe_response(SubscribeResponseMessageCRef &&message) {
  for (auto &it : _subscription_streams) {
    it.first->send_response(copy_ref_(message));
  }
}

void NodeState::check_subscribe_options() {
  SubscribeOptions new_options;
  for (auto &stream : _subscription_streams) {
    new_options.mergeFrom(stream.first->options());
  }
  if (new_options != _merged_subscribe_options && _model != nullptr) {
    if (new_options.is_empty()) {
      _model->unsubscribe();
    } else {
      // update options only
      _model->subscribe(new_options, nullptr);
    }
  }
}

void NodeState::subscribe(ref_<OutgoingSubscribeStream> &&stream) {
  auto p = stream.get();
  _subscription_streams[p] = std::move(stream);
  if (_merged_subscribe_options.mergeFrom(p->options())) {
    if (_model_status == MODEL_CONNECTED) {
      _model->subscribe(
          _merged_subscribe_options,
          [ this, keep_ref = get_ref() ](SubscribeResponseMessageCRef && msg) {
            new_subscribe_response(std::move(msg));
          });
    }
    // TODO update model;
  }
  p->on_option_change([ this, keep_ref = get_ref() ](
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
  if (_model != nullptr && _model->_cached_value != nullptr) {
    p->send_response(copy_ref_(_model->_cached_value));
  }
}

void NodeState::update_list_value(const std::string &key, BytesRef &value) {
  for (auto &it : _list_streams) {
    it.first->update_value(key, value);
  }
}

void NodeState::list(ref_<OutgoingListStream> &&stream) {
  auto p = stream.get();
  _list_streams[p] = std::move(stream);
  if (_model != nullptr) {
    _model->init_list_stream(*p);
  }
}

void NodeState::close_impl() {
  if (_model != nullptr) {
    _model->close();
    _model.reset();
    _model_status = MODEL_UNKNOWN;
    _owner.remove_state(_path.full_str());
  }
}

NodeStateChild::NodeStateChild(NodeStateOwner &owner, ref_<NodeState> &&parent,
                               const std::string &name)
    : NodeState(owner, std::move(parent)), name(name) {}

NodeStateRoot::NodeStateRoot(NodeStateOwner &owner, ref_<NodeModelBase> &&model)
    : NodeState(owner, ref_<NodeState>()) {
  _path = Path("");
  set_model(std::move(model));
};

}  // namespace dsa
