#ifndef DSA_SDK_STCP_CONNECTION_H
#define DSA_SDK_STCP_CONNECTION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>

#include "../base_socket_connection.h"
#include "util/enable_shared.h"

namespace dsa {

typedef boost::asio::ip::tcp::socket tcp_socket;
typedef boost::asio::ssl::stream<tcp_socket> ssl_socket;

// Secure TCP connection. Used for DSA connections over secure TCP.
// Handles DSA handshake, combining outgoing messages,
// and separating incoming messages.
class StcpConnection : public BaseSocketConnection {
  class WriteBuffer : public ConnectionWriteBuffer {
    StcpConnection &connection;
    size_t size = 0;

   public:
    explicit WriteBuffer(StcpConnection &connection) : connection(connection){};

    size_t max_next_size() const override;
    void add(const Message &msg, int32_t rid, int32_t ack_id) override;
    void write(WriteHandler &&callback) override;
  };

 protected:
  void continue_read_loop(shared_ptr_<Connection> &&sthis) final {
    start_read(std::move(sthis));
  }
  ssl_socket _socket;

  void destroy_impl() override;

 public:
  StcpConnection(LinkStrandRef &strand, boost::asio::ssl::context &context,
                 const string_ &dsid_prefix, const string_ &path = "");

  void start_read(shared_ptr_<Connection> &&connection) final;

  std::unique_ptr<ConnectionWriteBuffer> get_write_buffer() override;

  ssl_socket::lowest_layer_type &socket();
};

}  // namespace dsa

#endif  // DSA_SDK_STCP_CONNECTION_H
