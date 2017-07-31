#include "session.h"

#include <boost/bind.hpp>
#include <boost/thread/locks.hpp>

#include "responder/outgoing_message_stream.h"

namespace dsa {

Session::Session(BufferPtr session_id, const ConnectionPtr &connection)
    : _connection(connection), _session_id(std::move(session_id)) {}

Session::Session(const std::string &session_id, ConnectionPtr connection)
    : _connection(std::move(connection)), _session_id(std::make_shared<Buffer>(session_id)) {}

void Session::start() const {
  if (_connection == nullptr)
    throw std::runtime_error("Session started without connection");

  _connection->start();
}

void Session::stop() {
  if (_connection != nullptr) {
    _connection->close();
    _connection.reset();
  }
}

void Session::add_ready_outgoing_stream(uint32_t rid, size_t unique_id) {
  _strand->post(make_shared_this_lambda([=]() {
    _ready_streams.push(StreamInfo{rid, unique_id, &_outgoing_streams});
    if (!_is_writing) {
      _strand->post(boost::bind(&Session::_write_loop, shared_from_this()));
    }
  }));
}

bool Session::add_outgoing_subscription(const std::shared_ptr<OutgoingMessageStream> &stream) {
  boost::upgrade_lock<boost::shared_mutex> lock(_outgoing_key);
  if (_outgoing_streams.count(stream->_request_id) > 0)
    return false;

  {
    boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(lock);
    _outgoing_streams[stream->_request_id] = stream;
  }
  return true;
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

  auto buf = std::make_shared<Buffer>();
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
  _connection->write(buf, buf->size(), boost::bind(&Session::_write_loop, shared_from_this()));
}

}  // namespace dsa