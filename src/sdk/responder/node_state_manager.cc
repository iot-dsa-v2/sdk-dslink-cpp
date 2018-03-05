#include "dsa_common.h"

#include "node_state_manager.h"

#include <boost/algorithm/string.hpp>

#include "core/session.h"
#include "core/strand_timer.h"
#include "message/response/list_response_message.h"
#include "model_base.h"
#include "module/logger.h"
#include "node_model.h"
#include "stream/responder/outgoing_invoke_stream.h"
#include "stream/responder/outgoing_list_stream.h"
#include "stream/responder/outgoing_set_stream.h"
#include "stream/responder/outgoing_subscribe_stream.h"
#include "util/date_time.h"

namespace dsa {

static const Path PUB_PATH = Path("Pub");

NodeStateManager::NodeStateManager(LinkStrand &strand, ModelRef &&root_model,
                                   size_t timer_interval)
    : _root(new NodeStateRoot(*this, std::move(root_model))) {
  _timer =
      strand.add_timer(60000, [ this, keep_ref = get_ref() ](bool canceled) {
        return _on_timer(canceled);
      });
}
NodeStateManager::~NodeStateManager() = default;

void NodeStateManager::destroy_impl() {
  _root->destroy();
  _root.reset();
  for (auto it = _states.begin(); it != _states.end(); ++it)
    it->second->destroy();
  _states.clear();
  _timer->destroy();
}
bool NodeStateManager::_on_timer(bool canceled) {
  if (canceled || is_destroyed()) {
    return false;
  }
  if (_states.size() > _check_states_size_threshold) {
    // next check when size is bigger than the current size
    _check_states_size_threshold = _states.size();

    _root->periodic_check(DateTime::ms_since_epoch());

    if (_check_states_size_threshold > _states.size() * 2) {
      // do more check if lots of nodes can be cleared
      _check_states_size_threshold = _states.size() * 2;
    } else if (_check_states_size_threshold < _states.size() * 5 / 4) {
      // do less check if very few nodes can be cleared
      _check_states_size_threshold = _states.size() * 5 / 4;
    }
  } else if (_timer_skipped++ >= 30) {
    // make it checks at least once very 30 minutes
    _check_states_size_threshold = 0;
  }

  return true;
}

void NodeStateManager::remove_state(const string_ &path) {
  _states.erase(path);
}

ref_<NodeState> NodeStateManager::get_state(const Path &path) {
  if (path.is_root()) {
    return _root;
  }
  ref_<NodeState> state;

  auto search = _states.find(path.full_str());
  if (search != _states.end()) {
    state = search->second->get_ref();
  } else {
    state = _root->create_child(path, *_root, false);
    // register it in global map for quick access
    if (state != nullptr) {
      _states[path.full_str()] = state;
    }
  }

  if (state != nullptr && state->_model_status == NodeState::MODEL_UNKNOWN) {
    // check if we can attach model to the state
    auto parent = state->_parent;
    while (parent->_model_status != NodeState::MODEL_CONNECTED) {
      parent = parent->_parent;
    }
    state->set_model(parent->_model->on_demand_create_child(
        state->_path.rest_part(parent->_path)));
  }

  return std::move(state);
}
ref_<NodeState> NodeStateManager::check_state(const Path &path) {
  if (path.is_root()) {
    return _root->get_ref();
  }
  auto result = _states.find(path.full_str());
  if (result != _states.end()) {
    return result->second->get_ref();
  }
  return _root->find_child(path);
}

void NodeStateManager::model_added(const Path &path, ModelRef &model) {
  ref_<NodeState> state = check_state(path);
  if (state != nullptr) {
  }
}
void NodeStateManager::model_deleted(const Path &path) {}

void NodeStateManager::add(ref_<OutgoingSubscribeStream> &&stream) {
  ref_<NodeState> state = get_state(stream->path);
  if (state != nullptr) {
    state->subscribe(std::move(stream));
  } else {
    auto msg = make_ref_<SubscribeResponseMessage>();
    msg->set_status(MessageStatus::NOT_SUPPORTED);
    stream->send_subscribe_response(std::move(msg));
  }
}
void NodeStateManager::add(ref_<OutgoingListStream> &&stream) {
  ref_<NodeState> state = get_state(stream->path);
  if (state != nullptr) {
    state->list(std::move(stream));
  } else {
    // TODO send status
  }
}
void NodeStateManager::add(ref_<OutgoingInvokeStream> &&stream) {
  ref_<NodeState> state = get_state(stream->path);
  if (state != nullptr) {
    state->invoke(std::move(stream));
  } else {
    auto msg = make_ref_<InvokeResponseMessage>();
    msg->set_status(MessageStatus::NOT_SUPPORTED);
    stream->send_response(std::move(msg));
  }
}
void NodeStateManager::add(ref_<OutgoingSetStream> &&stream) {
  ref_<NodeState> state = get_state(stream->path);
  if (state != nullptr) {
    state->set(std::move(stream));
  } else {
    auto msg = make_ref_<SetResponseMessage>();
    msg->set_status(MessageStatus::NOT_SUPPORTED);
    stream->send_response(std::move(msg));
  }
}

ref_<NodeModel> NodeStateManager::get_profile(const string_ &path,
                                              bool dsa_standard) {
  ref_<NodeState> pub_state = get_state(PUB_PATH);
  if (dsa_standard) {
    NodeModel *pub_model = pub_state->model_cast<NodeModel>();
    if (pub_model == nullptr) {
      LOG_FATAL(__FILENAME__, LOG << "failed to create standard profile node");
    }
    // TODO implement a standard profile collection
  } else {
    auto state = pub_state->find_child(Path(path));
    if (state != nullptr) {
      return ref_<NodeModel>(state->model_cast<NodeModel>());
    }
  }

  return ref_<NodeModel>();
}

}  // namespace dsa
