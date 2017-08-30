#include "dsa_common.h"

#include "session.h"

#include <boost/bind.hpp>

#include "client.h"
#include "server.h"

namespace dsa {

Session::Session(LinkStrandPtr &strand, const std::string &session_id)
    : _strand(strand),
      _session_id(session_id),
      requester(*this),
      responder(*this) {}

void Session::connected(shared_ptr_<Connection> connection) {
  if (_connection != nullptr) {
    _connection->close();
  }
  _connection = std::move(connection);
}

void Session::close_impl() {
  if (_connection != nullptr) {
    _connection->close();
  }
}

void Session::disconnected(const shared_ptr_<Connection> &connection) {
  if (_connection.get() == connection.get()) {
    _connection.reset();
  }
}

void Session::receive_message(MessagePtr &&message) {
  if (message->is_request()) {
    // responder receive request and send response
    responder.receive_message(std::move(message));
  } else {
    // requester sent request and receive response
    requester.receive_message(std::move(message));
  }
}

intrusive_ptr_<MessageStream> Session::get_next_ready_stream() {
  while (!_ready_streams.empty()) {
    intrusive_ptr_<MessageStream> stream = std::move(_ready_streams.front());
    _ready_streams.pop_front();
    if (stream->peek_next_message_size(0) > 0) {
      return std::move(stream);
    }
  }
  return nullptr;
}

size_t Session::peek_next_message(size_t availible) {
  while (!_ready_streams.empty()) {
    intrusive_ptr_<MessageStream> &stream = _ready_streams.front();
    size_t size = stream->peek_next_message_size(availible);
    if (size > 0) {
      return size;
    }
    _ready_streams.pop_front();
  }
  return 0;
}

void Session::write_loop(intrusive_ptr_<Session> sthis) {
  Connection *connection = sthis->_connection.get();
  if (connection == nullptr) {
    sthis->_is_writing = false;
    return;
  }

  size_t next_message_size =
      sthis->peek_next_message(connection->max_buffer_size());
  if (next_message_size == 0) {
    sthis->_is_writing = false;
    return;
  }

  sthis->_is_writing = true;
  std::vector<uint8_t> &buf = connection->_write_buffer;

  size_t total_size = 0;
  while (next_message_size > 0 &&
         total_size < connection->preferred_buffer_size() &&
         total_size + next_message_size < connection->max_buffer_size()) {
    auto stream = sthis->get_next_ready_stream();
    MessagePtr message = stream->get_next_message();

    if (buf.size() < connection->max_buffer_size() &&
        total_size + message->size() > buf.size()) {
      buf.resize(buf.size() * 4);
    }

    message->write(&buf[total_size]);
    total_size += message->size();

    next_message_size =
        sthis->peek_next_message(connection->max_buffer_size() - total_size);
  }

  connection->write(
      buf.data(),
      total_size, [sthis = std::move(sthis)](
                      const boost::system::error_code &error) mutable {
        LinkStrandPtr strand = sthis->_strand;
        (*strand)()->dispatch([sthis = std::move(sthis)]() mutable {
          Session::write_loop(std::move(sthis));
        });
      });
}

void Session::add_ready_stream(intrusive_ptr_<MessageStream> stream) {
  _ready_streams.push_back(std::move(stream));
  if (!_is_writing) {
    write_loop(intrusive_this());
  }
}

}  // namespace dsa