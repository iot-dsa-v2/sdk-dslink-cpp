#include "dsa_common.h"

#include "node_state_manager.h"

#include <boost/algorithm/string.hpp>

#include "core/session.h"
#include "model_base.h"
#include "stream/responder/outgoing_invoke_stream.h"
#include "stream/responder/outgoing_list_stream.h"
#include "stream/responder/outgoing_set_stream.h"
#include "stream/responder/outgoing_subscribe_stream.h"
#include "message/response/list_response_message.h"

namespace dsa {

NodeStateManager::NodeStateManager(LinkStrand &strand, ModelRef &&root_model,
                                   size_t timer_interval)
    : _root(new NodeStateRoot(*this, std::move(root_model))),
      _timer(strand.get_io_context(),
             boost::posix_time::seconds(timer_interval)) {}

void NodeStateManager::destroy_impl() {
  _root->destroy();
  _root.reset();
  for (auto it = _states.begin(); it != _states.end(); ++it)
    it->second->destroy();
  _states.clear();
}

void NodeStateManager::remove_state(const string_ &path) {
  _states.erase(path);
}

ref_<NodeState> NodeStateManager::get_state(const Path &path) {
  if (path.is_root()) {
    return _root;
  }
  auto result = _states.find(path.full_str());
  if (result != _states.end()) {
    return result->second->get_ref();
  }

  ref_<NodeState> state = _root->create_child(path, *_root, false);
  // register it in global map for quick access
  if (state != nullptr) {
    _states[path.full_str()] = state;
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

}  // namespace dsa
