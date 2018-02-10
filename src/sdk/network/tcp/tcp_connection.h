#ifndef DSA_SDK_TCP_CONNECTION_H
#define DSA_SDK_TCP_CONNECTION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>

#include "../base_socket_connection.h"
#include "util/enable_shared.h"

namespace dsa {

typedef boost::asio::ip::tcp::socket tcp_socket;

// TCP connection. Used for DSA connections over TCP.
// Handles DSA handshake, combining outgoing messages,
// and separating incoming messages.
class TcpConnection : public BaseSocketConnection {
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
  void continue_read_loop(shared_ptr_<Connection> &&sthis) final {
    start_read(std::move(sthis));
  }
  tcp_socket _socket;

  void destroy_impl() override;

 public:
  TcpConnection(LinkStrandRef &strand, const string_ &dsid_prefix,
                const string_ &path = "");

  void start_read(shared_ptr_<Connection> &&connection) final;

  std::unique_ptr<ConnectionWriteBuffer> get_write_buffer() override;

  tcp_socket &socket();
};

}  // namespace dsa

#endif  // DSA_SDK_TCP_CONNECTION_H
