#include "tcp_connection.h"

#include <boost/bind.hpp>

namespace dsa {

TcpConnection::TcpConnection(const App &app)
    : Connection(app), _socket(app.io_service()), _strand(app.io_service()) {}

void TcpConnection::close() {
  _socket.close();
}

void TcpConnection::read_loop(size_t from_prev, const boost::system::error_code &error, size_t bytes_transferred) {
  if (!error && !destroyed()) {
    BufferPtr buf = std::move(_read_buffer);
    _read_buffer.reset(new Buffer());

    size_t total_bytes = from_prev + bytes_transferred;
    uint8_t *data = buf->data();
    size_t cur = 0;

    while (cur < total_bytes) {
      // always want full static header instead of just message size to make sure it's a valid message
      if (total_bytes - cur < StaticHeaderLength) {
        size_t partial_size = total_bytes - cur;
        _read_buffer->assign(&data[cur], partial_size);
        _socket.async_read_some(boost::asio::buffer(_read_buffer->data() + partial_size,
                                                    _read_buffer->capacity() - partial_size),
                                boost::bind(&TcpConnection::read_loop, share_this<TcpConnection>(), partial_size,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
        return;
      }

      StaticHeaders header(&data[cur]);
      if (header.message_size() < total_bytes - cur) {
        size_t partial_size = total_bytes - cur;

        // make sure buffer capacity is enough to read full message
        _read_buffer->resize(header.message_size());
        _read_buffer->assign(&data[cur], partial_size);

        // read the rest of the message
        boost::asio::async_read(_socket,
                                boost::asio::buffer(_read_buffer->data() + partial_size,
                                                    _read_buffer->capacity() - partial_size),
                                boost::bind(&TcpConnection::read_loop, share_this<TcpConnection>(), partial_size,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
        return;
      }

      // post job with message buffer
      _app.io_service().post(boost::bind(&TcpConnection::handle_read, share_this(),
                                         buf->get_message_buffer(cur, header.message_size())));

      cur += header.message_size();
    }

    _socket.async_read_some(boost::asio::buffer(_read_buffer->data(), _read_buffer->capacity()),
                            boost::bind(&TcpConnection::read_loop, share_this<TcpConnection>(), 0,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
  }
}

void TcpConnection::error_check_wrap(WriteCallback callback, const boost::system::error_code &error) {
  if (!error && !destroyed()) callback();
}

void TcpConnection::write(BufferPtr buf, size_t size, WriteCallback callback) {
  boost::asio::async_write(_socket, boost::asio::buffer(buf->data(), size),
                           boost::bind(&TcpConnection::error_check_wrap, share_this<TcpConnection>(), callback,
                                       boost::asio::placeholders::error));
}

tcp_socket &TcpConnection::socket() { return _socket; }

//////////////////////////////////////
// TcpServerConnection
//////////////////////////////////////
TcpServerConnection::TcpServerConnection(const App &app) : TcpConnection(app) {}

void TcpServerConnection::async_accept_connection_then_loop(const TcpServerPtr &server) {
  _server = server;
  connect();
}

void TcpServerConnection::connect() {
  _server->_acceptor.async_accept(_socket, boost::bind(&TcpServerConnection::start_handshake, share_this<TcpServerConnection>(),
                                                     boost::asio::placeholders::error));
}

void TcpServerConnection::continue_accept_loop(const boost::system::error_code &error) {
  // async_accept_connection_then_loop was used, continue the loop
  _app.io_service().dispatch(boost::bind(&TcpServer::accept_loop, _server));

  // start handshake
  start_handshake(error);
}

void TcpServerConnection::start_handshake(const boost::system::error_code &error) {
  if (error != nullptr) throw std::runtime_error("Client connection dropped unexpectedly");

  _socket.async_read_some(boost::asio::buffer(_read_buffer->data(), _read_buffer->capacity()),
                          boost::bind(&TcpServerConnection::f0_received, share_this<TcpServerConnection>(),
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred));

  size_t f1_size = load_f1(*_write_buffer);
  boost::asio::async_write(_socket, boost::asio::buffer(_write_buffer->data(), f1_size),
                           boost::bind(&TcpServerConnection::success_or_close, share_this<TcpServerConnection>(),
                                       boost::asio::placeholders::error));
}

void TcpServerConnection::f0_received(const boost::system::error_code &error, size_t bytes_transferred) {
  if (!error && !destroyed() && parse_f0(bytes_transferred)) {
    _strand.post(boost::bind(&TcpServerConnection::compute_secret, this));
    _socket.async_read_some(boost::asio::buffer(_read_buffer->data(), _read_buffer->capacity()),
                            boost::bind(&TcpServerConnection::f2_received, share_this<TcpServerConnection>(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
  }
}

void TcpServerConnection::f2_received(const boost::system::error_code &error, size_t bytes_transferred) {
  if (!error && !destroyed() && parse_f2(bytes_transferred)) {
    // wait for shared secret computation to finish
    _strand.post(boost::bind(&TcpServerConnection::send_f3, share_this<TcpServerConnection>()));
  }
}

void TcpServerConnection::send_f3() {
  // send f3 and start listening for requests
  size_t f3_size = load_f3(*_read_buffer);
  boost::asio::async_write(_socket, boost::asio::buffer(_read_buffer->data(), f3_size),
                           boost::bind(&TcpServerConnection::read_loop, share_this<TcpServerConnection>(), 0,
                                       boost::asio::placeholders::error, 0));
}

//////////////////////////////////
// TcpClientConnection
//////////////////////////////////
TcpClientConnection::TcpClientConnection(const App &app) : TcpConnection(app), config() {}

TcpClientConnection::TcpClientConnection(const App &app, const Config &config) : TcpConnection(app), config(config) {}

void TcpClientConnection::connect() {
  using tcp = boost::asio::ip::tcp;
  tcp::resolver resolver(_app.io_service());
  tcp::resolver::query query(config.host(), std::to_string(config.port()));
  tcp::endpoint endpoint = *resolver.resolve(query);
  _socket.async_connect(*resolver.resolve(query),
                        boost::bind(&TcpClientConnection::start_handshake, share_this<TcpClientConnection>(),
                                    boost::asio::placeholders::error));
}

void TcpClientConnection::start_handshake(const boost::system::error_code &error) {
  if (error != nullptr) throw std::runtime_error("Couldn't connect to specified host");

  _socket.async_read_some(boost::asio::buffer(_read_buffer->data(), _read_buffer->capacity()),
                          boost::bind(&TcpClientConnection::f1_received,
                                      share_this<TcpClientConnection>(),
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred));

  size_t f0_size = load_f0(*_write_buffer);
  boost::asio::async_write(_socket, boost::asio::buffer(_write_buffer->data(), f0_size),
                           boost::bind(&TcpClientConnection::success_or_close, share_this<TcpClientConnection>(),
                                       boost::asio::placeholders::error));
}

void TcpClientConnection::f1_received(const boost::system::error_code &error, size_t bytes_transferred) {
  if (!error && !destroyed() && parse_f1(bytes_transferred)) {
    // server should be parsing f0 and waiting for f2 at this point
    // so we can compute the shared secret synchronously
    compute_secret();
    size_t f2_size = load_f2(*_write_buffer);
    boost::asio::async_write(_socket, boost::asio::buffer(_write_buffer->data(), f2_size),
                             boost::bind(&TcpClientConnection::success_or_close,
                                         share_this<TcpClientConnection>(),
                                         boost::asio::placeholders::error));

    _socket.async_read_some(boost::asio::buffer(_read_buffer->data(), _read_buffer->capacity()),
                            boost::bind(&TcpClientConnection::f3_received,
                                        share_this<TcpClientConnection>(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
  }
}

void TcpClientConnection::f3_received(const boost::system::error_code &error, size_t bytes_transferred) {
  if (!error && !destroyed() && parse_f3(bytes_transferred)) {
    uint8_t *auth = _auth->data(), *other_auth = _other_auth->data();
    for (size_t i = 0; i < AuthLength; ++i)
      if (auth[i] != other_auth[i]) return;
    _socket.async_read_some(boost::asio::buffer(_read_buffer->data(), _read_buffer->capacity()),
                            boost::bind(&TcpClientConnection::read_loop, share_this<TcpClientConnection>(), 0,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
  }
}

}  // namespace dsa