#ifndef DSA_SDK_TCP_CONNECTION_H_
#define DSA_SDK_TCP_CONNECTION_H_

#include <boost/asio/ip/tcp.hpp>

#include "../connection.h"
#include "crypto/handshake_context.h"
#include "util/enable_shared.h"

namespace dsa {
class TcpServer;
class TcpClient;

typedef boost::asio::ip::tcp::socket tcp_socket;

// Base TCP connection. Used for DSA connections over TCP.
// Handles DSA handshake, combining outgoing messages,
// and separating incoming messages.
class TcpConnection : public Connection {
  static const uint32_t MAX_PENDING_MESSAGE = 2;

 protected:
  static void start_read(shared_ptr_<TcpConnection> &&connection,
                         size_t cur = 0, size_t next = 0);
  static void read_loop(shared_ptr_<TcpConnection> &&connection,
                        size_t from_prev,
                        const boost::system::error_code &error,
                        size_t bytes_transferred);
  tcp_socket _socket;
  std::atomic_bool _socket_open{true};

 public:
  TcpConnection(LinkStrandPtr &strand, uint32_t handshake_timeout_ms,
                const std::string &dsid_prefix, const std::string &path = "");

  void write_handler(WriteHandler callback,
                     const boost::system::error_code &error);

  void write(BufferPtr buf, size_t size, WriteHandler callback) override;

  tcp_socket &socket();

  void close_impl() override;
  void connect() override = 0;
  void start() throw() override;
};

}  // namespace dsa

#endif  // DSA_SDK_TCP_CONNECTION_H_
