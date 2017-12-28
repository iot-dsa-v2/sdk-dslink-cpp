#ifndef DSA_SDK_STCP_CONNECTION_H_
#define DSA_SDK_STCP_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>

#include "../connection.h"
#include "crypto/handshake_context.h"
#include "message/base_message.h"
#include "util/enable_shared.h"

namespace dsa {

typedef boost::asio::ip::tcp::socket tcp_socket;
typedef boost::asio::ssl::stream<tcp_socket> ssl_socket;

// Base TCP connection. Used for DSA connections over TCP.
// Handles DSA handshake, combining outgoing messages,
// and separating incoming messages.
class StcpConnection : public Connection {
  // write buffer will have 1/16 unusable part by default
  // which seems to improve the performance
  static const size_t DEFAULT_BUFFER_SIZE = 8192;
  static const size_t MAX_BUFFER_SIZE = DEFAULT_BUFFER_SIZE * 15;

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
  void read_loop_(shared_ptr_<Connection> &&connection, size_t from_prev,
                  const boost::system::error_code &error,
                  size_t bytes_transferred);
  void continue_read_loop(shared_ptr_<Connection> &&sthis) final {
    start_read(std::move(sthis));
  }
  std::vector<uint8_t> _read_buffer;
  std::vector<uint8_t> _write_buffer;
  ssl_socket _socket;
  std::atomic_bool _socket_open{true};

  void on_deadline_timer_(const boost::system::error_code &error,
                          shared_ptr_<Connection> &&sthis);

  void destroy_impl() override;

 public:
  StcpConnection(LinkStrandRef &strand, boost::asio::ssl::context &context,
                 const string_ &dsid_prefix, const string_ &path = "");

  void start_read(shared_ptr_<Connection> &&connection, size_t cur = 0,
                  size_t next = 0);

  std::unique_ptr<ConnectionWriteBuffer> get_write_buffer() override;

  ssl_socket::lowest_layer_type &socket();
};

}  // namespace dsa

#endif  // DSA_SDK_STCP_CONNECTION_H_
