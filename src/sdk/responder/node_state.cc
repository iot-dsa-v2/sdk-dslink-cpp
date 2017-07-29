#include "dsa_common.h"

#include <boost/bind.hpp>

#include "node_state.h"

namespace dsa {

NodeState::NodeState(boost::asio::io_service &io_service, std::string path)
    : _strand(io_service), _path(std::move(path)) {}

void NodeState::new_message(SubscribeResponseMessage message) {
  _strand.post(boost::bind(&NodeState::_handle_new_message, shared_from_this(), message));
}

void NodeState::_handle_new_message(SubscribeResponseMessage message) {
  _last_value.reset(new SubscribeResponseMessage(message));
  for (auto &stream : _subscription_streams)
    stream->new_message(message);
}

void NodeState::add_subscription_stream(std::shared_ptr<SubscribeMessageStream> stream) {
  auto shared_this = shared_from_this();
  _strand.post([=]() {
    shared_this->_subscription_streams.push_back(std::move(stream));
  });
}

}  // namespace dsa