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
class TcpConnection : public Connection {
 public:
  void read_loop(size_t from_prev, const boost::system::error_code &error, size_t bytes_transferred) override;
  tcp_socket _socket;
  boost::asio::io_service::strand _strand;

 public:
  explicit TcpConnection(const App &app);

  void error_check_wrap(WriteCallback callback, const boost::system::error_code &error);

  void write(BufferPtr buf, size_t size, WriteCallback callback) override;

  tcp_socket &socket();

  virtual void start() override;

  void close() override;
};

// TCP server side connection.
// Handles server side of DSA handshake and starts read loop.
class TcpServerConnection : public TcpConnection {
 private:
  void f0_received(const boost::system::error_code &error, size_t bytes_transferred);
  void f2_received(const boost::system::error_code &error, size_t bytes_transferred);
  void send_f3();

 public:
  explicit TcpServerConnection(const App &app);

  void start() override;
};

// TCP client side connection.
// Handles client side of DSA handshake and starts read loop.
class TcpClientConnection : public TcpConnection {
 public:
  struct Config {
   private:
    std::string _host{"127.0.0.1"};
    unsigned short _port{8080};

   public:
    void set_host(std::string host) { _host = host; }
    void set_port(unsigned short port) { _port = port; }
    const std::string &host() const { return _host; }
    unsigned short port() const { return _port; }
  };

  const Config config;

  explicit TcpClientConnection(const App &app);
  TcpClientConnection(const App &app, const Config &config);

  void start() override;
  void start_handshake(const boost::system::error_code &error);

 private:
  bool connect_to_host();
  void f1_received(const boost::system::error_code &error, size_t bytes_transferred);
  void f3_received(const boost::system::error_code &error, size_t bytes_transferred);
};

typedef std::shared_ptr<TcpServerConnection> TcpServerConnectionPtr;

}  // namespace dsa

#endif  // DSA_SDK_TCP_CONNECTION_H_
