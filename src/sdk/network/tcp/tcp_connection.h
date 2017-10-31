#ifndef DSA_SDK_TCP_CONNECTION_H_
#define DSA_SDK_TCP_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>

#include "../connection.h"
#include "crypto/handshake_context.h"
#include "message/base_message.h"
#include "util/enable_shared.h"

namespace dsa {
class TcpServer;
class Client;

typedef boost::asio::ip::tcp::socket tcp_socket;

// Base TCP connection. Used for DSA connections over TCP.
// Handles DSA handshake, combining outgoing messages,
// and separating incoming messages.
class TcpConnection : public Connection {

  // write buffer will have 1/16 unusable part by default
  // which seems to improve the performance
  static const size_t DEFAULT_BUFFER_SIZE = 8192;
  static const size_t MAX_BUFFER_SIZE = DEFAULT_BUFFER_SIZE * 15;

  class WriteBuffer : public ConnectionWriteBuffer {
    TcpConnection &connection;
    size_t size = 0;

   public:
    explicit WriteBuffer(TcpConnection &connection) : connection(connection){};

    size_t max_next_size() const override;
    void add(const Message &msg, int32_t rid, int32_t ack_id) override;
    void write(WriteHandler &&callback) override;
  };

 protected:
  void read_loop(shared_ptr_<TcpConnection> &&connection,
                        size_t from_prev,
                        const boost::system::error_code &error,
                        size_t bytes_transferred);

  std::vector<uint8_t> _read_buffer;
  std::vector<uint8_t> _write_buffer;
  tcp_socket _socket;
  std::atomic_bool _socket_open{true};

  void destroy_impl() override;

 public:
  TcpConnection(LinkStrandRef &strand,
                const string_ &dsid_prefix, const string_ &path = "");

  void start_read(shared_ptr_<TcpConnection> &&connection,
                         size_t cur = 0, size_t next = 0);

  std::unique_ptr<ConnectionWriteBuffer> get_write_buffer() override;

  tcp_socket &socket();
};

}  // namespace dsa

#endif  // DSA_SDK_TCP_CONNECTION_H_
