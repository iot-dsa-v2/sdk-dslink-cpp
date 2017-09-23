#include "dsa_common.h"

#include "tcp_client_connection.h"

#include "core/session.h"

namespace dsa {
TcpClientConnection::TcpClientConnection(LinkStrandRef &strand,
                                         uint32_t handshake_timeout_ms,
                                         const std::string &dsid_prefix,
                                         const std::string &tcp_host,
                                         uint16_t tcp_port,
                                         const std::string &path)
    : TcpConnection(strand, handshake_timeout_ms, dsid_prefix, path),
      _hostname(tcp_host),
      _port(tcp_port) {}

void TcpClientConnection::connect() {
  // connect to server
  using tcp = boost::asio::ip::tcp;
  tcp::resolver resolver(_strand->get_io_service());
  // TODO: timeout
  _socket.async_connect(
      *resolver.resolve(tcp::resolver::query(_hostname, std::to_string(_port))),
      // capture shared_ptr to keep the instance
      // capture this to access protected member
      [ connection = share_this<TcpConnection>(),
        this ](const boost::system::error_code &error) mutable {
        if (error != boost::system::errc::success) {
          TcpConnection::close_in_strand(std::move(connection));
          // TODO: log or return the error?
          return;
        }

        start_client_f0();

        TcpConnection::start_read(std::move(connection));
      });
}

// void TcpClientConnection::start_handshake(
//    const boost::system::error_code &error) throw(const std::runtime_error &)
//    {
//  if (error != boost::system::errc::errc_t::success) {
//    close();
//    throw std::runtime_error("Couldn't connect to specified host");
//  }
//
//#if 0
//  // TODO fix this
//  // start timeout timer
//  _deadline.expires_from_now(
//      boost::posix_time::milliseconds(_handshake_timeout_ms));
//
//
//  _deadline.async_wait(
//      boost::bind(&TcpClientConnection::timeout,
//                  Connection::share_this<TcpClientConnection>(),
//                  boost::asio::placeholders::error));
//#endif
//
//  _socket.async_read_some(
//      boost::asio::buffer(_write_buffer.data(), _write_buffer.capacity()),
//      boost::bind(&TcpClientConnection::f1_received,
//                  Connection::share_this<TcpClientConnection>(),
//                  boost::asio::placeholders::error,
//                  boost::asio::placeholders::bytes_transferred));
//
//  size_t f0_size = load_f0(_write_buffer);
//  boost::asio::async_write(
//      _socket, boost::asio::buffer(_write_buffer.data(), f0_size),
//      boost::bind(&TcpClientConnection::success_or_close,
//                  Connection::share_this<TcpClientConnection>(),
//                  boost::asio::placeholders::error));
//}
// void TcpClientConnection::f3_received(const boost::system::error_code &error,
//                                      size_t bytes_transferred) {
//  // start standard dsa 1 minute timeout
//  reset_standard_deadline_timer();
//
//  if (!error && parse_f3(bytes_transferred)) {
//    try {
//      Connection::on_client_connect();
//    } catch (const std::runtime_error &error) {
//#if DEBUG
//      std::stringstream ss;
//      ss << "[TcpClientConnection::f3_received] Error: " << error <<
//      std::endl;
//      std::cerr << ss.str();
//#endif
//      close();
//    }
//  } else {
//    close();
//  }
//}
}  // namespace dsa