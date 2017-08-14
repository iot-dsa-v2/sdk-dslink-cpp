#include "dsa_common.h"

#include <boost/bind.hpp>

#include "tcp_client_connection.h"

namespace dsa {
TcpClientConnection::TcpClientConnection(boost::asio::io_service::strand &strand,
                                         const Config &config, intrusive_ptr_<Session> session)
    : Connection(strand, config), TcpConnection(strand, config), ClientConnection(strand, config) {
  _session = std::move(session);
#if DEBUG
  std::cout << "TcpClientConnection()\n";
#endif
}

void TcpClientConnection::connect() {
  // connect to server
  using tcp = boost::asio::ip::tcp;
  tcp::resolver resolver(_strand.get_io_service());
  tcp::resolver::query query(_config.tcp_host, std::to_string(_config.tcp_port));
  tcp::endpoint endpoint = *resolver.resolve(query);
  _socket.async_connect(
      *resolver.resolve(query),
      boost::bind(&TcpClientConnection::start_handshake,
                  Connection::share_this<TcpClientConnection>(),
                  boost::asio::placeholders::error));
}

void TcpClientConnection::start_handshake(
    const boost::system::error_code &error) throw(const std::runtime_error &) {
  if (error != nullptr) {
    close();
    throw std::runtime_error("Couldn't connect to specified host");
  }

  // start timeout timer
  _deadline.expires_from_now(
      boost::posix_time::milliseconds(_config.handshake_timeout_ms));
  _deadline.async_wait(
      boost::bind(&TcpClientConnection::timeout,
                  Connection::share_this<TcpClientConnection>(),
                  boost::asio::placeholders::error));

  _socket.async_read_some(
      boost::asio::buffer(_read_buffer->data(), _read_buffer->capacity()),
      boost::bind(&TcpClientConnection::f1_received,
                  Connection::share_this<TcpClientConnection>(),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));

  size_t f0_size = load_f0(*_write_buffer);
  boost::asio::async_write(
      _socket, boost::asio::buffer(_write_buffer->data(), f0_size),
      boost::bind(&TcpClientConnection::success_or_close,
                  Connection::share_this<TcpClientConnection>(),
                  boost::asio::placeholders::error));
}

void TcpClientConnection::f1_received(const boost::system::error_code &error,
                                      size_t bytes_transferred) {
  if (!error && parse_f1(bytes_transferred)) {
    // cancel timer before timeout
    _deadline.expires_from_now(
        boost::posix_time::milliseconds(_config.handshake_timeout_ms));

    // server should be parsing f0 and waiting for f2 at this point
    // so we can compute the shared secret synchronously
    compute_secret();
    size_t f2_size = load_f2(*_write_buffer);
    boost::asio::async_write(
        _socket, boost::asio::buffer(_write_buffer->data(), f2_size),
        boost::bind(&TcpClientConnection::success_or_close,
                    Connection::share_this<TcpClientConnection>(),
                    boost::asio::placeholders::error));

    // restart timeout timer
    _deadline.async_wait(
        boost::bind(&TcpClientConnection::timeout,
                    Connection::share_this<TcpClientConnection>(),
                    boost::asio::placeholders::error));

    _socket.async_read_some(
        boost::asio::buffer(_read_buffer->data(), _read_buffer->capacity()),
        boost::bind(&TcpClientConnection::f3_received,
                    Connection::share_this<TcpClientConnection>(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  } else {
    close();
  }
}

void TcpClientConnection::f3_received(const boost::system::error_code &error,
                                      size_t bytes_transferred) {

  // start standard dsa 1 minute timeout
  reset_standard_deadline_timer();

  if (!error && parse_f3(bytes_transferred)) {
    // create new session object if none and pass to the on connect handler
    _session = make_intrusive_<Session>(_strand,
                                        _session_id,
                                        _config,
                                        Connection::shared_from_this());

    try {
      ClientConnection::on_connect();
    } catch (const std::runtime_error &error) {
#if DEBUG
      std::stringstream ss;
      ss << "[TcpClientConnection::f3_received] Error: " << error << std::endl;
      std::cerr << ss.str();
#endif
      close();
    }
  } else {
    close();
  }
}
}  // namespace dsa