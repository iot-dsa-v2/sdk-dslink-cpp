#include "dsa_common.h"

#include "tcp_server.h"

// TODO: remove this
#include <boost/asio.hpp>


#include "tcp_server_connection.h"

namespace dsa {

using tcp = boost::asio::ip::tcp;
TcpServer::TcpServer(WrapperConfig &config)
    : Server(config),
      _hostname(config.tcp_host),
      _port(config.tcp_port),
      _handshake_timeout_ms(config.handshake_timeout_ms),
      _acceptor(new tcp::acceptor((*_strand)().get_io_service(),
                                  tcp::endpoint(tcp::v4(), config.tcp_port))) {}

void TcpServer::start() {
  // start taking connections
  _new_connection = make_shared_<TcpServerConnection>(_strand, _handshake_timeout_ms);

  _acceptor->async_accept(
      _new_connection->socket(),
      boost::bind(&TcpServer::accept_loop, share_this<TcpServer>(),
                  boost::asio::placeholders::error));
}

void TcpServer::close() {
  _acceptor->close();
  // TODO: fix this!
  // Server::close();
}

void TcpServer::accept_loop(const boost::system::error_code &error) {
  if (!error) {
    _new_connection->connect();
    _new_connection = make_shared_<TcpServerConnection>(_strand, _handshake_timeout_ms);
    _acceptor->async_accept(
        _new_connection->socket(),
        boost::bind(&TcpServer::accept_loop, share_this<TcpServer>(),
                    boost::asio::placeholders::error));
  } else {
    close();
  }
}

}  // namespace dsa
