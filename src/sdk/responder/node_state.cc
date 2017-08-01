#include "dsa_common.h"

#include "node_state.h"

namespace dsa {

NodeState::NodeState(boost::asio::io_service &io_service, std::string path)
    : _strand(io_service), _path(std::move(path)) {

}

void NodeState::new_message(const SubscribeResponseMessage &message) {
  _strand.post(make_shared_this_lambda([=](){
    _last_value.reset(new SubscribeResponseMessage(message));
    for (auto &it : _subscription_streams)
      it.second->new_message(message);
  }));
}

void NodeState::add_subscription_stream(const std::shared_ptr<SubscribeMessageStream> &stream) {
  _strand.post(make_shared_this_lambda([=](){
    uint32_t rid = stream->_request_id;
    _subscription_streams[rid] = stream;
  }));
}

void NodeState::remove_subscription_stream(uint32_t request_id) {
  _strand.post(make_shared_this_lambda([=](){
    _subscription_streams.erase(request_id);
  }));
}

void NodeState::add_list_stream(std::shared_ptr<ListMessageStream> stream) {
  _strand.post(make_shared_this_lambda([=](){
    uint32_t rid = stream->_request_id;
    _list_streams[rid] = stream;
  }));
}

void NodeState::remove_list_stream(uint32_t request_id) {
  _strand.post(make_shared_this_lambda([=](){
    _subscription_streams.erase(request_id);
  }));
}

}  // namespace dsa