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
    return result->second->get_ref();
  } else if (create) {
    // create new node
    _children[name] = new NodeStateChild(_owner, get_ref(), name);
    return _children[name]->get_ref();
  }
  // not found, return a nullptr
  return ref_<NodeState>();
}
ref_<NodeState> NodeState::get_child(const Path &path, bool create) {
  const std::string &name = path.current_name();

  // find existing node
  auto result = _children.find(name);
  if (result != _children.end()) {
    if (path.is_last()) {
      return result->second->get_ref();
    }
    return result->second->get_child(path.next(), create);
  } else if (create) {
    // create new node
    _children[name] = new NodeStateChild(_owner, get_ref(), name);
    if (path.is_last()) {
      return _children[name]->get_ref();
    }
    return _children[name]->get_child(path.next(), true);
  }
  // not found, return a nullptr
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

void NodeState::check_model(const Path &path) {
  _path = path;
  if (_model == nullptr && _model_status == NodeState::MODEL_UNKNOWN) {
    if (_parent->_model_status > MODEL_CONNECTED) {
      _model_status = _parent->_model_status;
    } else {
      ref_<NodeState> &parent = _parent;
      Path p = path;
      while (parent->_model_status == MODEL_UNKNOWN) {
        parent = parent->_parent;
        p = p.previous();
      }
      if (parent->_model_status == MODEL_CONNECTED) {
        if (parent->_model->allows_on_demand_child()) {
          set_model(parent->_model->on_demand_create_child(p));
        }
      } else {
        // if parent status is  not valid, use same status
        _model_status = parent->_model_status;
      }
    }
  }
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
NodeStateChild::~NodeStateChild() {
  if (_path.data() != nullptr) {
    _owner.remove_state(_path.full_str());
  }
  _parent->remove_child(name);
}

NodeStateRoot::NodeStateRoot(NodeStateOwner &owner, ref_<NodeModel> &&model)
    : NodeState(owner) {
  set_model(std::move(model));
  // keep a ref so it won't be deleted by smart pointer
  // the instance will always be non-pointer member in NodeStateManager
  intrusive_ptr_add_ref(this);
};

}  // namespace dsa
