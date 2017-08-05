#ifndef DSA_SDK_TCP_CONNECTION_H_
#define DSA_SDK_TCP_CONNECTION_H_

#include <atomic>

#include <boost/asio.hpp>

#include "connection.h"
#include "crypto/handshake_context.h"
#include "tcp_server.h"
#include "util/enable_shared.h"

namespace dsa {

typedef boost::asio::ip::tcp::socket tcp_socket;

// Base TCP connection. Used for DSA connections over TCP.
// Handles DSA handshake, combining outgoing messages,
// and separating incoming messages.
class TcpConnection : public Connection {
 protected:
  const App *_app;
  void read_loop(size_t from_prev, const boost::system::error_code &error,
                 size_t bytes_transferred) override;
  tcp_socket _socket;
  std::atomic_bool _socket_open{true};

 public:
  TcpConnection(const App &app, const Config &config);

  void write_handler(WriteHandler callback,
                     const boost::system::error_code &error);

  void write(BufferPtr buf, size_t size, WriteHandler callback) override;

  tcp_socket &socket();

  virtual void name() = 0;
  void close() override;
  void connect() override = 0;
  void start() throw() override;
};

// TCP server side connection.
// Handles server side of DSA handshake and starts read loop.
class TcpServerConnection : public TcpConnection {
 private:
  void f0_received(const boost::system::error_code &error,
                   size_t bytes_transferred);
  void f2_received(const boost::system::error_code &error,
                   size_t bytes_transferred);
  void send_f3();

  // weak pointer needed here in order for the server to be able to be freed
  // once TcpServer::stop is called. std::weak_ptr::lock implementation just
  // copies a shared pointer so performance cost should be minimal. this pointer
  // should rarely be touched by connection.
  std::weak_ptr<TcpServer> _server;

 protected:
  void start_handshake();

 public:
  TcpServerConnection(const App &app, const Server::Config &config);
  ~TcpServerConnection() { std::cout << "~TcpServerConnection()\n"; }

  void connect() override;

  void name() override { std::cout << "TcpServerConnection\n"; }

  inline void set_server(shared_ptr_<TcpServer> server) {
    _server = std::move(server);
  };
};

// TCP client side connection.
// Handles client side of DSA handshake and starts read loop.
class TcpClientConnection : public TcpConnection {
 private:
  void f1_received(const boost::system::error_code &error,
                   size_t bytes_transferred);
  void f3_received(const boost::system::error_code &error,
                   size_t bytes_transferred);

 protected:
  void start_handshake(const boost::system::error_code &error);

 public:
  explicit TcpClientConnection(const App &app);
  TcpClientConnection(const App &app, const Config &config);
  ~TcpClientConnection() { std::cout << "~TcpClientConnection()\n"; }

  void name() override { std::cout << "TcpClientConnection\n"; }

  void connect() override;

  void close() override;

  intrusive_ptr_<Session> session() { return _session; }
};

typedef shared_ptr_<TcpServerConnection> TcpServerConnectionPtr;

}  // namespace dsa

#endif  // DSA_SDK_TCP_CONNECTION_H_
