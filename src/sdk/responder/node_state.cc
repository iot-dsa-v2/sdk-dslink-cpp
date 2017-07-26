#include "node_state.h"

#include <boost/bind.hpp>

namespace dsa {

NodeState::NodeState(boost::asio::io_service &io_service, std::string path)
    : _strand(io_service), _path(std::move(path)) {}

void NodeState::new_value(ValueUpdate &value) {
  _strand.post([=]() {
    _last_value.reset(new ValueUpdate(value));

  });
}

}  // namespace dsa