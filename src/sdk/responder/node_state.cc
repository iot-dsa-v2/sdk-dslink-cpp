#include "dsa_common.h"

#include "node_state.h"

#include "core/session.h"

#include "message/request/invoke_request_message.h"
#include "message/request/set_request_message.h"
#include "stream/responder/outgoing_invoke_stream.h"
#include "stream/responder/outgoing_list_stream.h"
#include "stream/responder/outgoing_set_stream.h"
#include "stream/responder/outgoing_subscribe_stream.h"

namespace dsa {

NodeStateWaitingCache::NodeStateWaitingCache() = default;
NodeStateWaitingCache::~NodeStateWaitingCache() = default;

NodeState::NodeState(NodeStateOwner &owner, ref_<NodeState> &&parent)
    : _owner(owner),
      _parent(std::move(parent)),
      _merged_subscribe_options(QosLevel::_0, -1) {}
NodeState::~NodeState() = default;

ref_<NodeState> NodeState::get_child(const string_ &name, bool create) {
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
  const string_ &name = path.current_name();

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
    ref_<NodeState> new_state = ref_<NodeState>(
        static_cast<NodeState *>(new NodeStateChild(_owner, get_ref(), name)));
    _children[name] = new_state;
    if (path.is_last()) {
      new_state->_path = path;
      if (_model_status == MODEL_UNKNOWN) {
        new_state->set_model(last_modeled_state._model->on_demand_create_child(
            path.move_pos(last_modeled_state._path.current_pos() + 1)));
      } else if (_model_status == MODEL_CONNECTED) {
        new_state->set_model(_model->on_demand_create_child(path));
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
  const string_ &name = path.current_name();

  auto result = _children.find(name);
  if (result != _children.end()) {
    if (path.is_last()) {
      return result->second;
    }

    return result->second->find_child(path.next());
  }
  return ref_<NodeState>();
}

void NodeState::remove_child(const string_ &name) { _children.erase(name); }

void NodeState::set_model(ModelRef &&model) {
  if (model == nullptr) {
    _model.reset();
    _model_status = MODEL_INVALID;
  } else if (model->_strand == nullptr) {  // Other Invalid Models
    _model.reset();
    if (model == NodeModelBase::WAITING) {
      _model_status = MODEL_WAITING;
      if (_waiting_cache == nullptr) {
        _waiting_cache.reset(new NodeStateWaitingCache());
      }
    } else if (model == NodeModelBase::UNAVAILABLE) {
      _model_status = MODEL_UNAVAILABLE;
    } else {
      _model_status = MODEL_INVALID;
    }
  } else {
    _model = std::move(model);
    _model->_state = get_ref();
    _model_status = MODEL_CONNECTED;
    _model->initialize();

    // send all waiting streams

    // subscribe streams
    if (!_merged_subscribe_options.is_invalid()) {
      _model->subscribe(_merged_subscribe_options);
      if (_model->_cached_value != nullptr) {
        for (auto &subscribe_stream : _subscription_streams) {
          subscribe_stream.first->send_subscribe_response(
              copy_ref_(_model->_cached_value));
        }
      }
    }
    // list streams
    for (auto &list_stream : _list_streams) {
      _model->list(*list_stream.first);
    }
    // invoke and set streams
    if (_waiting_cache != nullptr) {
      for (auto &invoke_stream : _waiting_cache->invokes) {
        _model->invoke(std::move(invoke_stream), _parent);
      }
      for (auto &set_stream : _waiting_cache->sets) {
        _model->set(std::move(set_stream));
      }
      _waiting_cache.reset();
    }
  }
}
bool NodeState::periodic_check(size_t ts) {
  for (auto it = _children.begin(); it != _children.end();) {
    if (it->second->periodic_check(ts)) {
      it = _children.erase(it);
    } else {
      it++;
    }
  }

  if (_subscription_streams.empty() && _list_streams.empty() &&
      _children.empty() &&
      // check if model is still in use
      (_model == nullptr || _model->periodic_check(ts))) {
    destroy();
    return true;
  }
  return false;
}

void NodeState::new_subscribe_response(SubscribeResponseMessageCRef &&message) {
  for (auto &it : _subscription_streams) {
    it.first->send_subscribe_response(copy_ref_(message));
  }
}

void NodeState::check_subscribe_options() {
  SubscribeOptions new_options(QosLevel::_0, -1);
  for (auto &stream : _subscription_streams) {
    new_options.mergeFrom(stream.first->subscribe_options());
  }
  if (new_options != _merged_subscribe_options) {
    _merged_subscribe_options = new_options;
    if (_model != nullptr) {
      if (new_options.is_invalid()) {
        _model->unsubscribe();
      } else {
        // update options only
        _model->subscribe(new_options);
      }
    }
  }
}

void NodeState::subscribe(ref_<BaseOutgoingSubscribeStream> &&stream) {
  auto p = stream.get();
  _subscription_streams[p] = std::move(stream);
  if (_model != nullptr && _model->_cached_value != nullptr) {
    p->send_subscribe_response(copy_ref_(_model->_cached_value));
  }
  if (_merged_subscribe_options.mergeFrom(p->subscribe_options())) {
    if (_model_status == MODEL_CONNECTED) {
      _model->subscribe(_merged_subscribe_options);
    }
    // TODO update model;
  }
  p->on_subscribe_option_change([
    this, keep_ref = get_ref()
  ](BaseOutgoingSubscribeStream & stream, const SubscribeOptions &old_options) {
    if (stream.is_destroyed()) {
      _subscription_streams.erase(&stream);
      if (_subscription_streams.empty() ||
          _merged_subscribe_options.needUpdateOnRemoval(
              stream.subscribe_options())) {
        check_subscribe_options();
      }
    } else {
      if (_merged_subscribe_options.needUpdateOnChange(
              old_options, stream.subscribe_options())) {
        check_subscribe_options();
      }
    }
  });
}

void NodeState::update_list_value(const string_ &key,
                                  const VarBytesRef &value) {
  for (auto &it : _list_streams) {
    it.first->update_list_value(key, value);
  }
}


void NodeState::update_list_status(MessageStatus status){
  for (auto &it : _list_streams) {
    it.first->update_list_status(status);
  }
}
void NodeState::update_list_refreshed(){
  for (auto &it : _list_streams) {
    it.first->update_list_refreshed();
  }
}
void NodeState::update_list_base_path(const string_ &path){
  for (auto &it : _list_streams) {
    it.first->update_list_base_path(path);
  }
}

void NodeState::list(ref_<BaseOutgoingListStream> &&stream) {
  auto p = stream.get();
  _list_streams[p] = std::move(stream);
  p->on_list_close([ this, keep_ref = get_ref() ](BaseOutgoingListStream & s) {
    _list_streams.erase(&s);
    if (_list_streams.empty() && _model != nullptr) {
      _model->unlist();
    }
  });
  if (_model != nullptr) {
    _model->list(*p);
  }
}

void NodeState::invoke(ref_<OutgoingInvokeStream> &&stream) {
  if (_model != nullptr) {
    _model->invoke(std::move(stream), _parent);
  } else if (_model_status == MODEL_WAITING) {
    _waiting_cache->invokes.emplace_back(std::move(stream));
  } else {
    auto response = make_ref_<InvokeResponseMessage>();
    if (_model_status == MODEL_UNAVAILABLE) {
      response->set_status(MessageStatus::DISCONNECTED);
    } else {
      response->set_status(MessageStatus::NOT_SUPPORTED);
    }
    stream->send_response(std::move(response));
  }
}
void NodeState::set(ref_<OutgoingSetStream> &&stream) {
  if (_model != nullptr) {
    _model->set(std::move(stream));
  } else if (_model_status == MODEL_WAITING) {
    _waiting_cache->sets.emplace_back(std::move(stream));
  } else {
    auto response = make_ref_<SetResponseMessage>();
    if (_model_status == MODEL_UNAVAILABLE) {
      response->set_status(MessageStatus::DISCONNECTED);
    } else {
      response->set_status(MessageStatus::NOT_SUPPORTED);
    }
    stream->send_response(std::move(response));
  }
}

void NodeState::destroy_impl() {
  if (_model != nullptr) {
    _model->destroy();
    _model.reset();
    _model_status = MODEL_UNKNOWN;
  }

  _subscription_streams.clear();
  _list_streams.clear();
  for (auto it = _children.begin(); it != _children.end(); ++it)
    it->second->destroy();
  _children.clear();
  if (_waiting_cache != nullptr) {
    _waiting_cache->invokes.clear();
    _waiting_cache->sets.clear();
    _waiting_cache.reset(nullptr);
  }

  if (!_path.is_invalid()) {
    _owner.remove_state(_path.full_str());
  }
  _parent.reset();
  // TODO: ali ask
  //  if(_parent != nullptr) {
  //    _parent->destroy();
  //    _parent.reset();
  //  }
}

NodeStateChild::NodeStateChild(NodeStateOwner &owner, ref_<NodeState> &&parent,
                               const string_ &name)
    : NodeState(owner, std::move(parent)), name(name) {}

NodeStateRoot::NodeStateRoot(NodeStateOwner &owner, ModelRef &&model)
    : NodeState(owner, ref_<NodeState>()) {
  _path = Path("");
  set_model(std::move(model));
};

}  // namespace dsa
