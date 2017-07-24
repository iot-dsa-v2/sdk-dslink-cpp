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

  auto shared_this = shared_from_this();
  _connection->set_read_handler([=](Buffer::SharedBuffer buf) {
    shared_this->message_handler(buf);
  });

  _connection->start();
}

void Session::stop() {
  if (_connection != nullptr) {
    _connection->close();
    _connection.reset();
  }
}

void Session::message_handler(SharedBuffer message_buffer) const {

}

}  // namespace dsa