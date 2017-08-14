#include "dsa_common.h"

#include "node_state.h"

#include "core/session.h"

namespace dsa {

NodeState::NodeState(boost::asio::io_service::strand &strand, std::string path)
    : _strand(strand), _path(std::move(path)) {}

void NodeState::new_message(const SubscribeResponseMessage &message) {
  _strand.post(make_intrusive_this_lambda([=]() {
    _last_value.reset(new SubscribeResponseMessage(message));
    for (auto &it : _subscription_streams) {
      auto &stream = dynamic_cast<SubscribeMessageStream &>(*it.second);
      stream.new_message(message);
    }
  }));
}

void NodeState::new_subscription_stream(const intrusive_ptr_<Session> &session,
                                        SubscribeOptions &&config,
                                        size_t unique_id,
                                        uint32_t request_id) {
  _subscription_streams[request_id] =
      make_intrusive_<SubscribeMessageStream>(session, &_subscription_streams, config, request_id, unique_id);
}

void NodeState::remove_subscription_stream(uint32_t request_id) {
  _subscription_streams.erase(request_id);
}

void NodeState::new_list_stream(const intrusive_ptr_<Session> &session,
                                ListOptions &&config,
                                size_t unique_id,
                                uint32_t request_id) {
  _list_streams[request_id] =
      make_intrusive_<ListMessageStream>(session, &_list_streams, config, request_id, unique_id);
}

void NodeState::remove_list_stream(uint32_t request_id) {
  _strand.post(make_intrusive_this_lambda([=]() {
    _subscription_streams.erase(request_id);
  }));
}

}  // namespace dsa