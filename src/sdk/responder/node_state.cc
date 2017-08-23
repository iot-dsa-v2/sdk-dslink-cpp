#include "dsa_common.h"

#include "node_state.h"

#include "core/session.h"

namespace dsa {

NodeState::NodeState(LinkStrandPtr & strand, const std::string &path)
    : strand(strand), _path(path) {}

void NodeState::new_message(const SubscribeResponseMessage &message) {
  _last_value.reset(new SubscribeResponseMessage(message));
  for (auto &it : _subscription_streams) {
    auto &stream = dynamic_cast<intrusive_ptr_<SubscribeMessageStream> &>(*it);
    stream->new_message(message);
  }
}

void NodeState::add_stream(intrusive_ptr_<SubscribeMessageStream> p) {
  _subscription_streams.insert(std::move(p));
}
void NodeState::add_stream(intrusive_ptr_<ListMessageStream> p) {
  _list_streams.insert(std::move(p));
}

void NodeState::remove_stream(intrusive_ptr_<SubscribeMessageStream> &p) {
  _subscription_streams.erase(std::move(p));
}
void NodeState::remove_stream(intrusive_ptr_<ListMessageStream> &p) {
  _list_streams.erase(std::move(p));
}

}  // namespace dsa