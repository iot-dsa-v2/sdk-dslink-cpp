#ifndef DSA_SDK_TCP_CONNECTION_H_
#define DSA_SDK_TCP_CONNECTION_H_

#include <boost/asio.hpp>

#include "connection.h"
#include "security_context.h"



#include "../util/enable_shared.h"

namespace dsa {

typedef boost::asio::ip::tcp::socket tcp_socket;

// Base TCP connection. Used for DSA connections over TCP.
// Handles DSA handshake, combining outgoing messages,
// and separating incoming messages.
class TcpConnection : public Connection {
 private:
  const SecurityContext &_security_context;
  tcp_socket _socket;

 protected:
  void read_loop(size_t from_prev, const boost::system::error_code &error, size_t bytes_transferred);

 public:
  TcpConnection(boost::asio::io_service &io_service, const SecurityContext &security_context);

  void error_check_wrap(WriteCallback callback, const boost::system::error_code &error);

  void write(BufferPtr buf, size_t size, WriteCallback callback);

  tcp_socket &socket();

  virtual void start();

  void close();
  void destroy();
};

// TCP server side connection.
// Handles server side of DSA handshake and starts read loop.
class TcpServerConnection : public TcpConnection {
 public:
  TcpServerConnection(boost::asio::io_service &io_service, const SecurityContext &security_context);

  void start();

  void f0_received(const boost::system::error_code &error, size_t bytes_transferred);
  void f1_sent(const boost::system::error_code &error, size_t bytes_transferred);
  void f2_received(const boost::system::error_code &error, size_t bytes_transferred);
  void f3_sent(const boost::system::error_code &error, size_t bytes_transferred);
};

// TCP client side connection.
// Handles client side of DSA handshake and starts read loop.
class TcpClientConnection : public TcpConnection {
 public:
  TcpClientConnection(boost::asio::io_service &io_service, const SecurityContext &security_context);

  void start();
};

typedef std::shared_ptr<TcpServerConnection> TcpServerConnectionPtr;

}  // namespace dsa

#endif  // DSA_SDK_TCP_CONNECTION_H_
