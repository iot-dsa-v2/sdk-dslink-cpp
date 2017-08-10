#include "dsa_common.h"

#include "session.h"

#include <boost/bind.hpp>
#include <boost/thread/locks.hpp>

#include "responder/outgoing_message_stream.h"

namespace dsa {

Session::Session(boost::asio::io_service::strand &strand, BufferPtr session_id, const shared_ptr_<Connection> &connection)
    : _connection(connection), _session_id(std::move(session_id)), _strand(strand) {}

Session::Session(boost::asio::io_service::strand &strand, const std::string &session_id, shared_ptr_<Connection> connection)
    : _connection(std::move(connection)), _session_id(make_intrusive_<Buffer>(session_id)), _strand(strand) {}

void Session::start() const {
  if (_connection == nullptr)
    throw std::runtime_error("Session started without connection");

  std::cout << "Session::start()" << std::endl;

  _connection->start();
}

void Session::close() {
  if (_connection != nullptr) {
    _connection->close();
  }
}

void Session::connection_closed() {
  _connection.reset();
}

void Session::add_ready_outgoing_stream(uint32_t rid, size_t unique_id) {
  _strand.post(make_intrusive_this_lambda([=]() {
    _ready_streams.push(StreamInfo{rid, unique_id, &_outgoing_streams});
    if (!_is_writing) {
      _strand.post(boost::bind(&Session::_write_loop, intrusive_this()));
    }
  }));
}

bool Session::add_outgoing_subscription(const shared_ptr_<OutgoingMessageStream> &stream) {
  boost::upgrade_lock<boost::shared_mutex> lock(_outgoing_key);
  if (_outgoing_streams.count(stream->_request_id) > 0)
    return false;

  {
    boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);
    _outgoing_streams[stream->_request_id] = stream;
  }
  return true;
}

void Session::remove_outgoing_subscription(uint32_t request_id) {
  boost::unique_lock<boost::shared_mutex> lock(_outgoing_key);
  _outgoing_streams.erase(request_id);
}

MessageStream *Session::_get_next_ready_stream() {
  while (!_ready_streams.empty()) {
    StreamInfo stream_info = _ready_streams.back();
    _ready_streams.pop();

    // make sure stream is still active
    if (stream_info.container->count(stream_info.rid) == 0) {
      continue;
    }

    // make sure stream info is accurate
    auto &stream = stream_info.container->at(stream_info.rid);
    if (stream->_unique_id == stream_info.unique_id) {
      return stream.get();
    }
  }
  return nullptr;
}

void Session::_write_loop() {
  if (_ready_streams.empty()) {
    _is_writing = false;
    return;
  }

  auto buf = make_intrusive_<Buffer>();
  MessageStream *stream = _get_next_ready_stream();

  if (stream == nullptr)
    return;

  // make sure buffer is big enough for at least the first message
  buf->resize(stream->get_next_message_size());

  do {
    buf->append(stream->get_next_message());
    stream = _get_next_ready_stream();
  } while (stream != nullptr && stream->get_next_message_size() + buf->size() < buf->capacity());

  _is_writing = true;
  _connection->write(buf, buf->size(), boost::bind(&Session::_write_loop, intrusive_this()));
}

}  // namespace dsa