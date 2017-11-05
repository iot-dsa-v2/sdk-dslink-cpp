#include "dsa_common.h"

#include "tcp_server.h"

#include "module/logger.h"
#include "tcp_server_connection.h"

namespace dsa {

using tcp = boost::asio::ip::tcp;
TcpServer::TcpServer(WrapperConfig &config)
    : Server(config),
      _port(config.tcp_port),
      _acceptor(
          new tcp::acceptor(_strand->get_io_service(),
                            // tcp:v6() already covers both ipv4 and ipv6
                            tcp::endpoint(tcp::v6(), config.tcp_server_port))) {
  LOG_INFO(_strand->logger(),
           LOG << "Bind to TCP server port: " << config.tcp_server_port);
}
TcpServer::~TcpServer() {
  if (!is_destroyed()) {
    destroy();
  }
}
void TcpServer::start() {
  // start taking connections
  _next_connection = make_shared_<TcpServerConnection>(_strand);

  _acceptor->async_accept(_next_connection->socket(), [
    this, sthis = shared_from_this()
  ](const boost::system::error_code &error) { accept_loop(error); });
}

void TcpServer::destroy_impl() {
  _acceptor->close();
  _next_connection->destroy();
  Server::destroy_impl();
}

void TcpServer::accept_loop(const boost::system::error_code &error) {
  if (!error) {
    _next_connection->accept();
    _next_connection = make_shared_<TcpServerConnection>(_strand);
    _acceptor->async_accept(_next_connection->socket(), [
      this, sthis = shared_from_this()
    ](const boost::system::error_code &err) { accept_loop(err); });
  } else {
    destroy();
  }
}

}  // namespace dsa
