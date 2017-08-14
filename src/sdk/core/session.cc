#include "dsa_common.h"

#include "session.h"

#include <boost/bind.hpp>

#include "responder/outgoing_message_stream.h"

#define DEBUG true

namespace dsa {

const std::string Session::BlankDsid = "";

Session::Session(boost::asio::io_service::strand &strand,
                 const std::string &session_id,
                 const Config &config,
                 const shared_ptr_<Connection> &connection)
    : _connection(connection),
      _config(config),
      _session_id(session_id),
      _strand(strand),
      requester(*this),
      responder(*this, config) {}

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

void Session::close() {
  if (_connection != nullptr) {
    _connection->close();
  }
}

void Session::connection_closed() {
  _connection.reset();
}

void Session::receive_message(Message *message) {
  if (message->is_request()) {
    auto new_message = intrusive_ptr_<RequestMessage>(dynamic_cast<RequestMessage*>(message));
    responder.receive_message(std::move(new_message));
  } else {
    auto new_message = intrusive_ptr_<ResponseMessage>(dynamic_cast<ResponseMessage*>(message));
    requester.receive_message(std::move(new_message));
  }
}

MessageStream *Session::get_next_ready_stream() {
  while (!_ready_streams.empty()) {
    MessageStream::StreamInfo stream_info = _ready_streams.back();
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

void Session::write_loop(intrusive_ptr_<Session> sthis) {
  if (sthis->_ready_streams.empty()) {
    sthis->_is_writing = false;
    return;
  }

  auto buf = make_intrusive_<Buffer>();
  MessageStream *stream = sthis->get_next_ready_stream();

  if (stream == nullptr)
    return;

  // make sure buffer is big enough for at least the first message
  buf->resize(stream->get_next_message_size());

  do {
    buf->append(stream->get_next_message());
    stream = sthis->get_next_ready_stream();
  } while (stream != nullptr && stream->get_next_message_size() + buf->size() < buf->capacity());

  sthis->_is_writing = true;
  sthis->_connection->write(buf, buf->size(), [strand = sthis->strand(), callback = [sthis = std::move(sthis)]() {
    write_loop(sthis);
  }]() mutable {
    strand.dispatch(callback);
  });
}

void Session::add_ready_stream(MessageStream::StreamInfo &&stream_info) {
  _ready_streams.push(stream_info);
  if (!_is_writing) {
    write_loop(intrusive_this());
  }
}

const std::string &Session::dsid() {
  return (_connection == nullptr) ? BlankDsid : _connection->dsid();
}

}  // namespace dsa