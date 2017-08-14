#include "dsa_common.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "tcp_server.h"

#include "tcp_server_connection.h"

namespace dsa {

using tcp = boost::asio::ip::tcp;

TcpServer::TcpServer(boost::asio::io_service::strand &strand, Config &config)
    : Server(strand, config),
      _acceptor(new tcp::acceptor(strand.get_io_service(),
                                  tcp::endpoint(tcp::v4(), config.tcp_port))) {}

void TcpServer::start() {
  // start taking connections
  _new_connection = make_shared_<TcpServerConnection>(_strand, _config);

  _acceptor->async_accept(
      _new_connection->socket(),
      boost::bind(&TcpServer::accept_loop, share_this<TcpServer>(),
                  boost::asio::placeholders::error));
}

void TcpServer::close() {
  _acceptor->close();
  Server::close();
}

void TcpServer::accept_loop(const boost::system::error_code &error) {
  if (!error) {
    _new_connection->set_server(share_this<TcpServer>());
    _new_connection->connect();
    _new_connection = make_shared_<TcpServerConnection>(_strand, _config);
    _acceptor->async_accept(
        _new_connection->socket(),
        boost::bind(&TcpServer::accept_loop, share_this<TcpServer>(),
                    boost::asio::placeholders::error));
  } else {
    close();
  }
}

}  // namespace dsa