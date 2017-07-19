#ifndef DSA_SDK_TCP_CONNECTION_H_
#define DSA_SDK_TCP_CONNECTION_H_

#include <memory>
#include <boost/asio.hpp>

#include "connection.h"
#include "security_context.h"

#include "util/enable_shared.h"

namespace dsa {

typedef boost::asio::ip::tcp::socket tcp_socket;

// Base TCP connection. Used for DSA connections over TCP.
// Handles DSA handshake, combining outgoing messages,
// and separating incoming messages.
class TcpConnection : public Connection  {
 public:
  void read_loop(size_t from_prev, const boost::system::error_code &error, size_t bytes_transferred);
  tcp_socket _socket;
  boost::asio::io_service::strand _strand;

 public:
  TcpConnection(const App &app);

  void error_check_wrap(WriteCallback callback, const boost::system::error_code &error);

  void write(BufferPtr buf, size_t size, WriteCallback callback);

  tcp_socket &socket();

  virtual void start();

  void close();
};

// TCP server side connection.
// Handles server side of DSA handshake and starts read loop.
class TcpServerConnection : public TcpConnection {
 private:
  void f0_received(const boost::system::error_code &error, size_t bytes_transferred);
  void f2_received(const boost::system::error_code &error, size_t bytes_transferred);
  void send_f3();

 public:
  TcpServerConnection(const App &app);

  void start();
};

// TCP client side connection.
// Handles client side of DSA handshake and starts read loop.
class TcpClientConnection : public TcpConnection {
 public:
  TcpClientConnection(const App &app);

  void start();

  void f1_received(const boost::system::error_code &error, size_t bytes_transferred);
  void f3_received(const boost::system::error_code &error, size_t bytes_transferred);
};

typedef std::shared_ptr<TcpServerConnection> TcpServerConnectionPtr;

}  // namespace dsa

#endif  // DSA_SDK_TCP_CONNECTION_H_
