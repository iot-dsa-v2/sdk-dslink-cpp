#include "dsa_common.h"

#include "node_state.h"

#include "core/session.h"

namespace dsa {

NodeStateBase::NodeStateBase(boost::asio::io_service::strand &strand, const std::string &path)
    : _strand(strand), _path(path) {}

void NodeStateBase::new_message(const SubscribeResponseMessage &message) {
  _strand.post(make_intrusive_this_lambda([=]() {
    _last_value.reset(new SubscribeResponseMessage(message));
    for (auto &it : _subscription_streams) {
      auto &stream = dynamic_cast<SubscribeMessageStream &>(*it.second);
      stream.new_message(message);
    }
  }));
}

void NodeStateBase::new_subscription_stream(const intrusive_ptr_<Session> &session,
                                        SubscribeOptions &&config,
                                        size_t unique_id,
                                        uint32_t request_id) {
  _subscription_streams[request_id] =
      make_intrusive_<SubscribeMessageStream>(session, &_subscription_streams, config, request_id, unique_id);
}

void NodeStateBase::remove_subscription_stream(uint32_t request_id) {
  _subscription_streams.erase(request_id);
}

void NodeStateBase::new_list_stream(const intrusive_ptr_<Session> &session,
                                ListOptions &&config,
                                size_t unique_id,
                                uint32_t request_id) {
  _list_streams[request_id] =
      make_intrusive_<ListMessageStream>(session, &_list_streams, config, request_id, unique_id);
}

void NodeStateBase::remove_list_stream(uint32_t request_id) {
  _strand.post(make_intrusive_this_lambda([=]() {
    _subscription_streams.erase(request_id);
  }));
}

NodeState::NodeState(boost::asio::io_service::strand &strand,
const std::string &path):NodeStateBase(strand, path){}

}  // namespace dsa