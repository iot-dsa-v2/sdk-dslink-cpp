#include "dsa_common.h"

#include "session.h"

#include <boost/bind.hpp>

#include "server.h"
#include "client.h"
#include "responder/outgoing_message_stream.h"

#define DEBUG true

namespace dsa {

const std::string Session::BlankDsid = "";

Session::Session(boost::asio::io_service::strand &strand,
                 const std::string &session_id, 
                 SecurityManager &security_manager,
                 NodeModelManager &model_manager,
                 NodeStateManager &state_manager,
                 const shared_ptr_<Connection> &connection)
    : _strand(strand),
      _session_id(session_id),
      _connection(connection),
      requester(*this),
      responder(*this, security_manager, model_manager, state_manager) {}

Session::Session(const Server &server, const std::string &session_id,
                 const shared_ptr_<Connection> &connection)
    : Session(server.get_strand(), session_id, server.get_security_manager(),
              server.get_node_model_manager(),
              server.get_node_state_manager()) {}

Session::Session(const Client &client, const std::string &session_id,
                 const shared_ptr_<Connection> &connection)
    : Session(client.get_strand(), session_id, client.get_security_manager(),
              client.get_node_model_manager(),
              client.get_node_state_manager()) {}

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

intrusive_ptr_<MessageStream> Session::get_next_ready_stream() {
  while (!_ready_streams.empty()) {
    auto stream = _ready_streams.back();
    _ready_streams.pop();
    if (!stream->is_closed())
      return std::move(stream);
  }
  return nullptr;
}

void Session::write_loop(intrusive_ptr_<Session> sthis) {
  if (sthis->_ready_streams.empty()) {
    sthis->_is_writing = false;
    return;
  }

  auto buf = make_intrusive_<ByteBuffer>();
  auto stream = sthis->get_next_ready_stream();

  if (stream == nullptr)
    return;

  // make sure buffer is big enough for at least the first message
  buf->resize(stream->get_next_message_size());

  do {
    // append message data to buffer then resize
    auto& message = stream->get_next_message();
    buf->resize(buf->size() + message.size());
    message.write(&buf->operator[](buf->size()));

    stream = sthis->get_next_ready_stream();
  } while (stream != nullptr && stream->get_next_message_size() + buf->size() < buf->capacity());

  sthis->_is_writing = true;
  sthis->_connection->write(buf, buf->size(), [strand = sthis->strand(), callback = [sthis = std::move(sthis)]() {
    write_loop(sthis);
  }]() mutable {
    strand.dispatch(callback);
  });
}

void Session::add_ready_stream(intrusive_ptr_<MessageStream> stream) {
  _ready_streams.push(std::move(stream));
  if (!_is_writing) {
    write_loop(intrusive_this<Session>());
  }
}

const std::string &Session::dsid() {
  return (_connection == nullptr) ? BlankDsid : _connection->dsid();
}

}  // namespace dsa