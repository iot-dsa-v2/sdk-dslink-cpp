#include "node_state.h"

namespace dsa {

NodeState::NodeState(boost::asio::io_service &io_service, std::string path)
    : _strand(io_service), _path(std::move(path)) {}

void NodeState::add_session(std::shared_ptr<Session> session) {
  _strand.post([=]() {
    _sessions.push_back(session);
  });
}

}  // namespace dsa