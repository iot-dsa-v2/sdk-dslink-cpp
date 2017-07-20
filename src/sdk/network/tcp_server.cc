#include "tcp_server.h"

#include "tcp_connection.h"

namespace dsa {

using tcp = boost::asio::ip::tcp;

TcpServer::TcpServer(const App &app, const Config &config)
    : Server(app), _config(config),
      _acceptor(new tcp::acceptor(app.io_service(), tcp::endpoint(tcp::v4(), config.port()))) {}

void TcpServer::start() {
  // start taking connections
  accept_loop();
}

void TcpServer::accept_loop() {
//  if (!destroyed()) {
  auto new_connection = std::make_shared<TcpServerConnection>(_app);

  // this will call accept_loop() once new connection is accepted
  new_connection->async_accept_connection_then_loop(share_this<TcpServer>());
//  }
}

}  // namespace dsa