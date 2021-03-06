#include "dsa_common.h"

#include "tcp_server.h"

#include "module/logger.h"
#include "stcp_server_connection.h"
#include "tcp_server_connection.h"

#include <iostream>

namespace dsa {

using tcp = boost::asio::ip::tcp;
TcpServer::TcpServer(WrapperStrand &config)
    : Server(config),
      _port(config.tcp_port),
      _secure_port(config.tcp_secure_port) {
  try {
    _acceptor = make_unique_<boost::asio::ip::tcp::acceptor>(tcp::acceptor(
        _shared_strand->get_io_context(),
        tcp::endpoint(boost::asio::ip::address::from_string(config.server_host),
                      config.tcp_server_port),
        false));  // TODO: true means port reusable
  } catch (boost::exception &e) {
    LOG_FATAL(__FILENAME__,
              LOG << "Bind Error: server port is already in use\n");
  }

  // It means auto select so update port
  if (_port == 0) {
    _port = _acceptor->local_endpoint().port();
  }

  LOG_FINE(__FILENAME__,
           LOG << "Bind to TCP server port: " << config.tcp_server_port);

  // secure tcp
  if (_secure_port < 0) {
    LOG_FINE(__FILENAME__, LOG << "Secure TCP is disabled\n");
    return;
  }

  ssl::context ssl_context{ssl::context::sslv23};
  boost::system::error_code error_code;
  if (!load_server_certificate(ssl_context, error_code)) {
    _secure_port = -1;
    return;
  }

  try {
    _secure_acceptor = make_unique_<boost::asio::ip::tcp::acceptor>(
        tcp::acceptor(_shared_strand->get_io_context(),
                      tcp::endpoint(boost::asio::ip::address::from_string(
                                        config.server_host),
                                    _secure_port),
                      false));

  } catch (boost::system::system_error &e) {
    LOG_ERROR(__FILENAME__, LOG << "Bind Error: " << e.what() << "\n");
    return;
  }

  // It means auto select so update port
  if (_secure_port == 0) {
    _secure_port = _secure_acceptor->local_endpoint().port();
  }
  LOG_FINE(__FILENAME__,
           LOG << "Bind to TCP secure server port: " << _secure_port << "\n");
}
TcpServer::~TcpServer() {
  if (!is_destroyed()) {
    LOG_ERROR(__FILENAME__, LOG << "server deleted before destroyed");
    destroy();
  }
}
void TcpServer::start() {
  // start taking connections
  _next_connection =
      make_shared_<TcpServerConnection>(_shared_strand, _dsid_prefix);

  _acceptor->async_accept(
      _next_connection->socket(),
      [this, sthis = shared_from_this()](
          const boost::system::error_code &error) { accept_loop(error); });

  // start taking secure connection
  if (_secure_port < 0) {
    return;
  }

  // if (!_secure_acceptor->is_open()) return;
  _secure_next_connection =
      make_shared_<StcpServerConnection>(_shared_strand, _dsid_prefix);

  _secure_acceptor->async_accept(_secure_next_connection->socket(),
                                 [this, sthis = shared_from_this()](
                                     const boost::system::error_code &error) {
                                   secure_accept_loop(error);
                                 });
}

void TcpServer::destroy_impl() {
  _acceptor->close();
  _next_connection->destroy();
  _next_connection.reset();
  Server::destroy_impl();

  if (_secure_port > 0) {
    _secure_acceptor->close();
    _secure_next_connection->destroy();
    _secure_next_connection.reset();
  }
}

void TcpServer::accept_loop(const boost::system::error_code &error) {
  DSA_REF_GUARD;
  if (!error) {
    _next_connection->accept();
    _next_connection = make_shared_<TcpServerConnection>(_shared_strand);
    _acceptor->async_accept(
        _next_connection->socket(),
        [this, sthis = shared_from_this()](
            const boost::system::error_code &err) { accept_loop(err); });
  } else {
    destroy();
  }
}

void TcpServer::secure_accept_loop(const boost::system::error_code &error) {
  DSA_REF_GUARD;
  if (!error) {
    _secure_next_connection->accept();
    _secure_next_connection =
        make_shared_<StcpServerConnection>(_shared_strand);
    _secure_acceptor->async_accept(
        _secure_next_connection->socket(),
        [this, sthis = shared_from_this()](
            const boost::system::error_code &err) { secure_accept_loop(err); });
  } else {
    destroy();
  }
}

int TcpServer::get_port() { return _port; }
int TcpServer::get_secure_port() { return _secure_port; }

}  // namespace dsa
