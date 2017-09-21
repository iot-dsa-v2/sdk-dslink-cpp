#include "dsa_common.h"

#include "node_state.h"

#include "core/session.h"

#include "stream/responder/outgoing_list_stream.h"
#include "stream/responder/outgoing_subscribe_stream.h"

namespace dsa {

NodeState::NodeState(NodeStateOwner &owner) : _owner(owner) {}

ref_<NodeState> NodeState::get_child(const std::string &name, bool create) {
  // find existing node
  auto result = _children.find(name);
  if (result != _children.end()) {
    return result->second;
  } else if (create) {
    // create new node
    _children[name] = new NodeStateChild(_owner, get_ref(), name);
    return _children[name];
  }
  // not found, return a nullptr
  return ref_<NodeState>();
}
ref_<NodeState> NodeState::create_child(const Path &path,
                                        NodeState &last_modeled_state) {
  const std::string &name = path.current_name();

  // find existing node
  auto result = _children.find(name);
  if (result != _children.end()) {
    if (path.is_last()) {
      return result->second;
    }
    NodeState &modeled_state = result->second->_model != nullptr
                                   ? *result->second
                                   : last_modeled_state;
    return result->second->create_child(path.next(), modeled_state);
  } else if (last_modeled_state._model->allows_runtime_child_change()) {
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
    return _children[name]->create_child(path.next(), last_modeled_state);
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

void NodeState::set_model(ref_<NodeModel> &&model) {
  if (model == nullptr) {
    _model.reset();
    if (model == NodeModel::WAITING) {
      _model_status = MODEL_WAITING;
    } else if (model == NodeModel::UNAVAILABLE) {
      _model_status = MODEL_UNAVAILABLE;
    } else {
      _model_status = MODEL_INVALID;
    }
  } else {
    _model = std::move(model);
    _model->_state = get_ref();
    _model_status = MODEL_CONNECTED;
  }
  // TODO send request to model
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
  _subscription_streams[stream.get()] = stream;
  if (_merged_subscribe_options.mergeFrom(stream->options())) {
    if (_model_status == MODEL_CONNECTED) {
      _model->subscribe(
          _merged_subscribe_options,
          [ this, keep_ref = get_ref() ](SubscribeResponseMessageCRef && msg) {
            new_subscribe_response(std::move(msg));
          });
    }
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
  if (_model != nullptr && _model->_cached_value != nullptr) {
    stream->send_response(copy_ref_(_model->_cached_value));
  }
}
void NodeState::list(ref_<OutgoingListStream> &&stream) {
  //_list_streams.insert(std::move(p));
}

NodeStateChild::NodeStateChild(NodeStateOwner &owner, ref_<NodeState> parent,
                               const std::string &name)
    : NodeState(owner), _parent(std::move(parent)), name(name) {}

NodeStateRoot::NodeStateRoot(NodeStateOwner &owner, ref_<NodeModel> &&model)
    : NodeState(owner) {
  set_model(std::move(model));
};

}  // namespace dsa
