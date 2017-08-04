#include "dsa_common.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "tcp_server.h"

#include "tcp_connection.h"

namespace dsa {

using tcp = boost::asio::ip::tcp;

TcpServer::TcpServer(const App &app, const Config &config)
    : Server(app), _config(config),
      _acceptor(new tcp::acceptor(app.io_service(), tcp::endpoint(tcp::v4(), config.port()))) {}

void TcpServer::start() {
//  register_this();

  // start taking connections
//  _new_connection = make_intrusive_<TcpServerConnection>(*_app, _config);
//
//  _acceptor->async_accept(_new_connection->socket(),
//                          boost::bind(&TcpServer::accept_loop,
//                                      share_this<TcpServer>(),
//                                      boost::asio::placeholders::error));
}

void TcpServer::close() {
  _acceptor->close();
  Server::close();
}

void TcpServer::accept_loop(const boost::system::error_code &error) {
  if (!error) {
    _new_connection->set_server(share_this<TcpServer>());
    _new_connection->connect();
    _new_connection.reset(new TcpServerConnection(*_app, _config));
    _acceptor->async_accept(_new_connection->socket(),
                            boost::bind(&TcpServer::accept_loop,
                                        share_this<TcpServer>(),
                                        boost::asio::placeholders::error));
  } else {
    close();
  }
}

}  // namespace dsa