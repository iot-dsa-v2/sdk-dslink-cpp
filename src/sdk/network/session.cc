#include "session.h"

namespace dsa {

std::atomic_long Session::_session_count{0};

Session::Session(std::shared_ptr<Connection> connection)
    : _connection(std::move(connection)), _session_id(_session_count++) {}

void Session::start() const {

}

}  // namespace dsa