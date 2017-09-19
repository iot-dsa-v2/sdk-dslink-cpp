#include "dsa_common.h"

#include "tcp_server.h"

#include <boost/asio/strand.hpp>

#include "tcp_server_connection.h"

namespace dsa {

using tcp = boost::asio::ip::tcp;
TcpServer::TcpServer(WrapperConfig &config)
    : Server(config),
      _hostname(config.tcp_host),
      _port(config.tcp_port),
      _handshake_timeout_ms(config.handshake_timeout_ms),
      _acceptor(new tcp::acceptor(
          static_cast<boost::asio::strand *>(_strand->asio_strand())
              ->get_io_service(),
          tcp::endpoint(tcp::v4(), config.tcp_port))) {}
TcpServer::~TcpServer() {
  if (!is_closed()) {
    close();
  }
}
void TcpServer::start() {
  // start taking connections
  _next_connection =
      make_shared_<TcpServerConnection>(_strand, _handshake_timeout_ms);

  _acceptor->async_accept(_next_connection->socket(), [
    this, sthis = shared_from_this()
  ](const boost::system::error_code &error) { accept_loop(error); });
}

void TcpServer::close_impl() {
  _acceptor->close();
  // TODO: fix this!
  _next_connection->close();
  Server::close_impl();
}

void TcpServer::accept_loop(const boost::system::error_code &error) {
  if (!error) {
    _next_connection->accept();
    _next_connection =
        make_shared_<TcpServerConnection>(_strand, _handshake_timeout_ms);
    _acceptor->async_accept(_next_connection->socket(), [
      this, sthis = shared_from_this()
    ](const boost::system::error_code &err) { accept_loop(err); });
  } else {
    close();
  }
}

}  // namespace dsa
