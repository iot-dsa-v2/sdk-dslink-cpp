#include "dsa_common.h"

#include "tcp_server_connection.h"

#include <boost/bind.hpp>

// TODO: remove this
#include <boost/asio.hpp>

#include "tcp_server.h"

#define DEBUG 0

namespace dsa {
TcpServerConnection::TcpServerConnection(LinkStrandPtr & strand,
                                         uint32_t handshake_timeout_ms,
                                         const std::string &dsid_prefix,
                                         const std::string &path)
    : TcpConnection(strand, handshake_timeout_ms, dsid_prefix,
                    path) {}

void TcpServerConnection::connect() { start_handshake(); }

void TcpServerConnection::start_handshake() {
#if 0
  // TODO fix this
  // start timeout timer with handshake timeout specified in config
  _deadline.expires_from_now(
      boost::posix_time::milliseconds(_handshake_timeout_ms));
  _deadline.async_wait(boost::bind(&TcpServerConnection::timeout,
                                   share_this<TcpServerConnection>(),
                                   boost::asio::placeholders::error));
#endif

  // start listening for f0
  _socket.async_read_some(
      boost::asio::buffer(_write_buffer.data(), _write_buffer.capacity()),
      boost::bind(&TcpServerConnection::f0_received,
                  share_this<TcpServerConnection>(),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));

  // prepare and send f1 then make sure it was successful
  // [success_or_close(...)]
  size_t f1_size = load_f1(_write_buffer);
  print("f1_write", f1_size);
  boost::asio::async_write(_socket,
                           boost::asio::buffer(_write_buffer.data(), f1_size),
                           boost::bind(&TcpServerConnection::success_or_close,
                                       share_this<TcpServerConnection>(),
                                       boost::asio::placeholders::error));
}

void TcpServerConnection::f0_received(const boost::system::error_code &error,
                                      size_t bytes_transferred) {
// reset timeout
#if 0
  // TODO fix this
  _deadline.expires_from_now(
      boost::posix_time::milliseconds(_handshake_timeout_ms));
  _deadline.async_wait(boost::bind(&TcpServerConnection::timeout,
                                   share_this<TcpServerConnection>(),
                                   boost::asio::placeholders::error));
#endif

  if (!error && parse_f0(bytes_transferred)) {
    // compute shared secret
    compute_secret();

    // read and goto -> f2_received()
    _socket.async_read_some(
        boost::asio::buffer(_write_buffer.data(), _write_buffer.capacity()),
        boost::bind(&TcpServerConnection::f2_received,
                    share_this<TcpServerConnection>(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  } else {
    close();
  }
}

void TcpServerConnection::f2_received(const boost::system::error_code &error,
                                      size_t bytes_transferred) {
  // start dsa standard 1 minute timeout
  reset_standard_deadline_timer();

  if (!error && parse_f2(bytes_transferred)) {
    try {
      Connection::on_server_connect();
    } catch (const std::runtime_error &error) {
#if DEBUG
      std::stringstream ss;
      ss << "[TcpServerConnection::f2_received] error: " << error << std::endl;
      std::cerr << ss.str();
#endif
      close();
      return;
    }

    // send f3 once everything is successful
    send_f3();
  } else {
    close();
  }
}

void TcpServerConnection::send_f3() {
  // send f3
  size_t f3_size = load_f3(_write_buffer);
  boost::asio::async_write(
      _socket, boost::asio::buffer(_write_buffer.data(), f3_size),
      boost::bind(&TcpServerConnection::success_or_close, shared_from_this(),
                  boost::asio::placeholders::error));
}

}  // namespace dsa
