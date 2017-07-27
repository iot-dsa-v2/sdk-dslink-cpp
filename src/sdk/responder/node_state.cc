#include "dsa_common.h"

#include "node_state.h"

#include <boost/bind.hpp>

namespace dsa {

NodeState::NodeState(boost::asio::io_service &io_service, std::string path)
    : _strand(io_service), _path(std::move(path)) {}

void NodeState::new_value(ValueUpdate &value) {
  _strand.post(boost::bind(&NodeState::_handle_new_value, shared_from_this(), value));
}

void NodeState::_handle_new_value(ValueUpdate value) {
  _last_value.reset(new ValueUpdate(value));
  for (auto &stream : _subscription_streams)
    stream->new_value(value);
}

}  // namespace dsa