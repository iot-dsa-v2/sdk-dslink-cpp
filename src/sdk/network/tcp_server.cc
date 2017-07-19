#include "tcp_server.h"

#include "tcp_connection.h"

namespace dsa {

TcpServer::TcpServer(const App &app, unsigned short port)
    : Server(app), _acceptor(app.io_service(), boost::asio::ip::tcp::endpoint(
    boost::asio::ip::tcp::v4(), port)) {}

void TcpServer::start() {
  // start taking connections
  accept_loop();
}

void TcpServer::accept_loop() {
  if (!destroyed()) {
    auto new_connection = std::make_shared<TcpServerConnection>(_app);

    // this will call accept_loop() once new connection is accepted
    new_connection->async_accept_connection_then_loop(share_this<TcpServer>());
  }
}

}  // namespace dsa