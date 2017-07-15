#ifndef DSA_SDK_TCP_CONNECTION_H_
#define DSA_SDK_TCP_CONNECTION_H_

#include <boost/asio.hpp>

#include "connection.h"
#include "security_context.h"
#include "dsa/util.h"

namespace dsa {

typedef boost::asio::ip::tcp::socket tcp_socket;

// Base TCP connection. Used for DSA connections over TCP.
// Handles DSA handshake, combining outgoing messages,
// and separating incoming messages.
class TcpConnection {
 private:
  const SecurityContext &_security_context;
  tcp_socket _socket;

 protected:
  Buffer _buffer;
  BufferPtr _shared_secret;
  BufferPtr _other_public_key;
  BufferPtr _other_salt;
  BufferPtr _other_dsid;
  BufferPtr _other_auth;
  BufferPtr _other_token;
  std::string path;

  // parse handshake messages
  bool parse_f0(uint8_t *data);
  bool parse_f1(uint8_t *data);
  bool parse_f2(uint8_t *data);
  bool parse_f3(uint8_t *data);

  // load handshake messages
  bool load_f0(Buffer &buf);
  bool load_f1(Buffer &buf);
  bool load_f2(Buffer &buf);
  bool load_f3(Buffer &buf);

  void read_loop(const boost::system::error_code &error, size_t bytes_transferred);

 public:
  TcpConnection(boost::asio::io_service &io_service, const SecurityContext &security_context);

  void error_check_wrap(WriteCallback callback, const boost::system::error_code &error);

  void write(BufferPtr buf, size_t size, WriteCallback callback);

  tcp_socket &socket();

  virtual void start();

  void close();
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

typedef std::shared_ptr<TcpConnection> TCPConnectionPtr;

}  // namespace dsa

#endif  // DSA_SDK_TCP_CONNECTION_H_
