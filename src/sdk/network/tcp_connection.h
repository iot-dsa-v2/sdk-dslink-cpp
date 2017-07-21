#ifndef DSA_SDK_TCP_CONNECTION_H_
#define DSA_SDK_TCP_CONNECTION_H_

#include <memory>

#include <boost/asio.hpp>

#include "util/enable_shared.h"
#include "connection.h"
#include "security_context.h"
#include "tcp_server.h"

namespace dsa {

typedef boost::asio::ip::tcp::socket tcp_socket;

// Base TCP connection. Used for DSA connections over TCP.
// Handles DSA handshake, combining outgoing messages,
// and separating incoming messages.
class TcpConnection : virtual public Connection {
 protected:
  void read_loop(size_t from_prev, const boost::system::error_code &error, size_t bytes_transferred) override;
  tcp_socket _socket;
  boost::asio::io_service::strand _strand;

 public:
  explicit TcpConnection(const App &app, const Config &config);

  void error_check_wrap(WriteHandler callback, const boost::system::error_code &error);

  void write(BufferPtr buf, size_t size, WriteHandler callback) override;

  tcp_socket &socket();

  void close() override;
  void connect() override = 0;
  void start() throw() override;
};

// TCP server side connection.
// Handles server side of DSA handshake and starts read loop.
class TcpServerConnection : public TcpConnection {
 private:
  void f0_received(const boost::system::error_code &error, size_t bytes_transferred);
  void f2_received(const boost::system::error_code &error, size_t bytes_transferred);
  void send_f3();

  // weak pointer needed here in order for the server to be able to be freed once TcpServer::stop is called.
  // std::weak_ptr::lock implementation just copies a shared pointer so performance
  // cost should be minimal. this pointer should rarely be touched by connection.
  std::weak_ptr<TcpServer> _server;

 protected:
  void start_handshake();

 public:
  explicit TcpServerConnection(const App &app, const Server::Config &config);

  void connect() override;

  inline void set_server(std::shared_ptr<TcpServer> server) { _server = std::move(server); };
};

// TCP client side connection.
// Handles client side of DSA handshake and starts read loop.
class TcpClientConnection : public TcpConnection {
 private:
  void f1_received(const boost::system::error_code &error, size_t bytes_transferred);
  void f3_received(const boost::system::error_code &error, size_t bytes_transferred);

 protected:
  void start_handshake(const boost::system::error_code &error);

 public:
  explicit TcpClientConnection(const App &app);
  TcpClientConnection(const App &app, const Config &config);

  void connect() override;

};

typedef std::shared_ptr<TcpServerConnection> TcpServerConnectionPtr;

}  // namespace dsa

#endif  // DSA_SDK_TCP_CONNECTION_H_
