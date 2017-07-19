#include "tcp_server.h"

#include <boost/bind.hpp>

namespace dsa {

TcpServer::TcpServer(const App &app, unsigned short port)
    : Server(app), _acceptor(app.io_service(), boost::asio::ip::tcp::endpoint(
    boost::asio::ip::tcp::v4(), port)) {}

void TcpServer::start() {
  auto connection = std::make_shared<TcpServerConnection>(_app);
  _acceptor.async_accept(connection->socket(),
                         boost::bind(&TcpServer::accept_loop, this, std::move(connection),
                                     boost::asio::placeholders::error));
}

void TcpServer::accept_loop(TcpServerConnectionPtr connection, const boost::system::error_code &error) {
  connection->connect();
  auto new_connection = std::make_shared<TcpServerConnection>(_app);
  _acceptor.async_accept(connection->socket(),
                         boost::bind(&TcpServer::accept_loop, this, std::move(connection),
                                     boost::asio::placeholders::error));
}

}  // namespace dsa