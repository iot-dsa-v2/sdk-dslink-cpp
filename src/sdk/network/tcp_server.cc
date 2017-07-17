#include "tcp_server.h"

#include <boost/bind.hpp>

namespace dsa {

TcpServer::TcpServer(boost::asio::io_service &io_service, SecurityContextPtr &security_context, unsigned short port)
    : Server(security_context), _io_service(io_service), _acceptor(io_service, boost::asio::ip::tcp::endpoint(
    boost::asio::ip::tcp::v4(), port)) {}

void TcpServer::start() {
  auto connection = std::make_shared<TcpServerConnection>(_io_service, security_context());
//  auto connection = new TcpServerConnection(_io_service, security_context());
  _acceptor.async_accept(connection->socket(),
                         boost::bind(&TcpServer::accept_loop, this, std::move(connection),
                                     boost::asio::placeholders::error));
}

void TcpServer::accept_loop(TcpServerConnectionPtr connection, const boost::system::error_code &error) {
  connection->start();
  auto new_connection = std::make_shared<TcpServerConnection>(_io_service, security_context());
  _acceptor.async_accept(connection->socket(),
                         boost::bind(&TcpServer::accept_loop, this, std::move(connection),
                                     boost::asio::placeholders::error));
}

}  // namespace dsa