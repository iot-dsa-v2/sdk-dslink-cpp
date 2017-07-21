#include "session.h"

#include <utility>

namespace dsa {

Session::Session(BufferPtr session_id, const ConnectionPtr &connection)
    : _connection(connection), _session_id(std::move(session_id)) {}

Session::Session(const std::string &session_id, ConnectionPtr connection)
    : _connection(std::move(connection)), _session_id(std::make_shared<Buffer>(session_id)) {}

void Session::start() const {
  if (_connection == nullptr)
    throw std::runtime_error("Session started without connection");

  // TODO: implement this
  std::cout << "Session start" << std::endl;
//  _connection->start();
}

void Session::stop() {
  _connection.reset();
}

}  // namespace dsa