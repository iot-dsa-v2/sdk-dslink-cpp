#include "dsa_common.h"

#include "node_state.h"

#include "core/session.h"

#include "stream/responder/outgoing_list_stream.h"
#include "stream/responder/outgoing_subscribe_stream.h"

namespace dsa {

NodeState::NodeState(LinkStrandRef &strand, const std::string &path)
    : strand(strand), _path(path) {}

ref_<NodeState> &NodeState::get_child(const Path &path) {
  throw std::runtime_error("get_child is not implemented!");
}

void NodeState::set_model(ModelRef model) { _model = std::move(model); }

void NodeState::new_message(ref_<SubscribeResponseMessage> &&message) {
  _last_value = message;
  for (auto &it : _subscription_streams) {
    auto &stream = dynamic_cast<ref_<OutgoingSubscribeStream> &>(*it);
    stream->send_message(std::move(message));
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

}  // namespace dsa
