#ifndef DSA_SDK_TCP_CONNECTION_H_
#define DSA_SDK_TCP_CONNECTION_H_

#include <atomic>

#include <boost/asio.hpp>

#include "core/connection.h"
#include "crypto/handshake_context.h"
#include "network/tcp/tcp_server.h"
#include "util/enable_shared.h"

namespace dsa {

typedef boost::asio::ip::tcp::socket tcp_socket;

// Base TCP connection. Used for DSA connections over TCP.
// Handles DSA handshake, combining outgoing messages,
// and separating incoming messages.
class TcpConnection : virtual public Connection {

  static const uint32_t MAX_PENDING_MESSAGE = 2;

 protected:
  const App *_app;
  void read_loop(size_t from_prev, const boost::system::error_code &error,
                 size_t bytes_transferred) override;
  tcp_socket _socket;
  std::atomic_bool _socket_open{true};

 public:
  TcpConnection(boost::asio::io_service::strand &strand, const Config &config);

  void write_handler(WriteHandler callback,
                     const boost::system::error_code &error);

  void write(BufferPtr buf, size_t size, WriteHandler callback) override;

  tcp_socket &socket();
  
  void close() override;
  void connect() override = 0;
  void start() throw() override;
};

}  // namespace dsa

#endif  // DSA_SDK_TCP_CONNECTION_H_
