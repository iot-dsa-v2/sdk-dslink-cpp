#include "dsa_common.h"

#include "session.h"

#include <boost/bind.hpp>

#include "client.h"
#include "server.h"

#define DEBUG true

namespace dsa {

Session::Session(LinkStrandPtr &strand, const std::string &session_id,
                 const shared_ptr_<Connection> &connection)
    : _strand(strand),
      _session_id(session_id),
      _connection(connection),
      requester(*this),
      responder(*this) {}

void Session::start() const {
  if (_connection == nullptr)
    throw std::runtime_error("Session started without connection");

#if DEBUG
  std::stringstream ss;
  ss << _connection->name() << " -> Session::start()" << std::endl;
  std::cout << ss.str();
#endif

  _connection->start();
}

void Session::close_impl() {
  if (_connection != nullptr) {
    _connection->close();
  }
}

void Session::connection_closed() { _connection.reset(); }

void Session::receive_message(Message *message) {
  if (message->is_request()) {
    auto new_message =
        intrusive_ptr_<RequestMessage>(dynamic_cast<RequestMessage *>(message));
    responder.receive_message(std::move(new_message));
  } else {
    auto new_message = intrusive_ptr_<ResponseMessage>(
        dynamic_cast<ResponseMessage *>(message));
    requester.receive_message(std::move(new_message));
  }
}

intrusive_ptr_<MessageStream> Session::get_next_ready_stream() {
  while (!_ready_streams.empty()) {
    intrusive_ptr_<MessageStream> stream = std::move(_ready_streams.front());
    _ready_streams.pop_front();
    if (stream->peek_next_message_size() > 0) {
      return std::move(stream);
    }
  }
  return nullptr;
}

size_t Session::peek_next_message() {
  while (!_ready_streams.empty()) {
    intrusive_ptr_<MessageStream> &stream = _ready_streams.front();
    size_t size = stream->peek_next_message_size();
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

  size_t next_message_size = sthis->peek_next_message();
  if (next_message_size == 0) {
    sthis->_is_writing = false;
    return;
  }

  sthis->_is_writing = true;
  std::vector<uint8_t> &buf = connection->_write_buffer;

  size_t total_size = 0;
  while (next_message_size > 0 &&
         total_size < Connection::DEFAULT_BUFFER_SIZE &&
         total_size + next_message_size < Connection::MAX_BUFFER_SIZE) {
    auto stream = sthis->get_next_ready_stream();
    MessagePtr message = stream->get_next_message();

    if (buf.size() < Connection::MAX_BUFFER_SIZE &&
        total_size + message->size() > buf.size()) {
      buf.resize(buf.size() * 4);
    }

    message->write(&buf[total_size]);
    total_size += message->size();

    next_message_size = sthis->peek_next_message();
  }

  connection->write(
      buf.data(),
      total_size, [sthis = std::move(sthis)](
                      const boost::system::error_code &error) mutable {
        LinkStrandPtr strand = sthis->_strand;
        (*strand)().dispatch([sthis = std::move(sthis)]() mutable {
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