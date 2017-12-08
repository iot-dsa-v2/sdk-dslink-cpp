#include "dsa_common.h"

#include "tcp_server.h"

#include "module/logger.h"
#include "tcp_server_connection.h"

namespace dsa {

using tcp = boost::asio::ip::tcp;
TcpServer::TcpServer(WrapperStrand &config)
    : Server(config),
      _port(config.tcp_port) {

  try {
    _acceptor = make_unique_<boost::asio::ip::tcp::acceptor>(tcp::acceptor(
        _strand->get_io_service(),
        tcp::endpoint(
            boost::asio::ip::address::from_string(config.server_host),
            config.tcp_server_port), false));
  } catch (boost::exception &e) {
    LOG_FATAL(LOG << "Bind Error: server port is already in use\n");
  }

  // It means auto select so update port
  if (_port == 0) {
    _port = _acceptor->local_endpoint().port();
  }
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
  _next_connection.reset();
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
int TcpServer::get_port() {
  return _port;
}

}  // namespace dsa
