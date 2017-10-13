#include "dsa_common.h"

#include "node_state_manager.h"

#include <boost/algorithm/string.hpp>

#include "core/session.h"
#include "model_base.h"
#include "stream/responder/outgoing_list_stream.h"
#include "stream/responder/outgoing_subscribe_stream.h"

namespace dsa {

NodeStateManager::NodeStateManager(LinkStrand &strand,
                                   ModelRef &&root_model,
                                   size_t timer_interval)
    : _root(new NodeStateRoot(*this, std::move(root_model))),
      _timer(strand.get_io_service(),
             boost::posix_time::seconds(timer_interval)) {}

void NodeStateManager::remove_state(const std::string &path) {
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
  _states[path.full_str()] = state;
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
  state->subscribe(std::move(stream));
}
void NodeStateManager::add(ref_<OutgoingListStream> &&stream) {
  ref_<NodeState> state = get_state(stream->path);
  state->list(std::move(stream));
}
void NodeStateManager::add(ref_<OutgoingInvokeStream> &&stream) {}
void NodeStateManager::add(ref_<OutgoingSetStream> &&stream) {}

}  // namespace dsa