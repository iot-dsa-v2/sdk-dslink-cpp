#ifndef DSA_SDK_TCP_SERVER_H_
#define DSA_SDK_TCP_SERVER_H_

#include <boost/asio.hpp>

#include "server.h"
#include "tcp_connection.h"
#include "app.h"

namespace dsa {
class TcpServer : public Server {
 private:
  boost::asio::ip::tcp::acceptor _acceptor;
  boost::asio::io_service & _io_service;

  void accept_loop(TcpServerConnectionPtr connection, const boost::system::error_code &error);

 public:
  TcpServer(App &app, unsigned short port);

  void start();
};
}  // namespace dsa

#endif  // DSA_SDK_TCP_SERVER_H_
