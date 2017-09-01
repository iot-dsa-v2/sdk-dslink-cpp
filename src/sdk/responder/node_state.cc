#include "dsa_common.h"

#include "node_state.h"

#include "core/session.h"

namespace dsa {

NodeState::NodeState(LinkStrandRef &strand, const std::string &path)
    : strand(strand), _path(path) {}

void NodeState::set_model(ModelRef model) { _model = std::move(model); }

void NodeState::new_message(const SubscribeResponseMessage &message) {
  _last_value.reset(new SubscribeResponseMessage(message));
  for (auto &it : _subscription_streams) {
    auto &stream = dynamic_cast<ref_<OutgoingSubscribeStream> &>(*it);
    stream->new_message(message);
  }
}

void NodeState::add_stream(ref_<OutgoingSubscribeStream> p) {
  _subscription_streams.insert(std::move(p));
}
void NodeState::add_stream(ref_<OutgoingListStream> p) {
  _list_streams.insert(std::move(p));
}

void NodeState::remove_stream(ref_<OutgoingSubscribeStream> &p) {
  _subscription_streams.erase(std::move(p));
}
void NodeState::remove_stream(ref_<OutgoingListStream> &p) {
  _list_streams.erase(std::move(p));
}

}  // namespace dsa