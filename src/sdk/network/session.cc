#include "session.h"

#include <functional>

namespace dsa {

Session::Session(BufferPtr session_id, const ConnectionPtr &connection)
    : _connection(connection), _session_id(std::move(session_id)) {}

Session::Session(const std::string &session_id, ConnectionPtr connection)
    : _connection(std::move(connection)), _session_id(std::make_shared<Buffer>(session_id)) {}

void Session::start() const {
  if (_connection == nullptr)
    throw std::runtime_error("Session started without connection");

  std::cout << "session start" << std::endl;
  std::string session_id = "test/session/id";
  Session new_session(session_id);
  auto handler = std::bind(&Session::stop, &new_session);
  // TODO: implement this
//  _connection->set_read_handler(std::bind(&Session::message_handler, this, std::placeholders::_1));
//  _connection->start();
}

void Session::stop() {
  if (_connection != nullptr) {
    _connection->close();
    _connection.reset();
  }
}

void Session::message_handler(SharedBuffer message_buffer) {

}

}  // namespace dsa