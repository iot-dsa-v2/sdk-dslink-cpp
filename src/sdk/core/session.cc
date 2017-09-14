#include "dsa_common.h"

#include "session.h"

#include "client.h"
#include "server.h"

#include "stream/ack_stream.h"

namespace dsa {

Session::Session(LinkStrandRef strand, const std::string &session_id)
    : _strand(std::move(strand)),
      _session_id(session_id),
      requester(*this),
      responder(*this),
      _ack_stream(new AckStream(get_ref())) {}

Session::~Session() = default;

void Session::connected(shared_ptr_<Connection> connection) {
  if (_connection != nullptr) {
    _connection->close();
  }
  _connection = std::move(connection);
}

void Session::close_impl() {
  requester.close_impl();
  responder.close_impl();
  if (_connection != nullptr) {
    _connection->close();
  }
  _ack_stream.reset();
}

void Session::disconnected(const shared_ptr_<Connection> &connection) {
  if (_connection.get() == connection.get()) {
    _connection.reset();
  }
}

void Session::receive_message(MessageRef &&message) {
  LOG_TRACE(_strand->logger(), LOG<< "receive message: " << message->type());
  if (message->need_ack()) {
    _ack_stream->add_ack(message->get_ack_id());
  }
  if (message->is_request()) {
    // responder receive request and send response
    responder.receive_message(std::move(message));
  } else {
    // requester sent request and receive response
    requester.receive_message(std::move(message));
  }
}

ref_<MessageStream> Session::get_next_ready_stream() {
  while (!_write_streams.empty()) {
    ref_<MessageStream> stream = std::move(_write_streams.front());
    _write_streams.pop_front();
    if (stream->peek_next_message_size(0) > 0) {
      return std::move(stream);
    }
  }
  return nullptr;
}

size_t Session::peek_next_message(size_t availible) {
  while (!_write_streams.empty()) {
    ref_<MessageStream> &stream = _write_streams.front();
    size_t size = stream->peek_next_message_size(availible);
    if (size > 0) {
      return size;
    }
    _write_streams.pop_front();
  }
  return 0;
}

void Session::write_loop(ref_<Session> sthis) {
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
    MessageCRef message = stream->get_next_message(sthis->_next_ack);

    if (buf.size() < connection->max_buffer_size() &&
        total_size + message->size() > buf.size()) {
      buf.resize(buf.size() * 4);
    }

    LOG_TRACE(sthis->_strand->logger(), LOG<< "send message: " << message->type());

    message->write(&buf[total_size], stream->rid, sthis->_next_ack++);
    total_size += message->size();

    next_message_size =
        sthis->peek_next_message(connection->max_buffer_size() - total_size);
  }

  connection->write(
      buf.data(),
      total_size, [sthis = std::move(sthis)](
                      const boost::system::error_code &error) mutable {
        LinkStrandRef strand = sthis->_strand;
        (*strand)()->dispatch([sthis = std::move(sthis)]() mutable {
          Session::write_loop(std::move(sthis));
        });
      });
}

void Session::write_stream(ref_<MessageStream> &&stream) {
  _write_streams.push_back(std::move(stream));
  if (!_is_writing) {
    write_loop(get_ref());
  }
}

}  // namespace dsa