#ifndef DSA_SDK_WS_CONNECTION_H_
#define DSA_SDK_WS_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/websocket.hpp>

#include "../connection.h"
#include "crypto/handshake_context.h"
#include "message/base_message.h"
#include "util/enable_shared.h"

namespace dsa {

  //using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>
  //namespace websocket =
    boost::beast::websocket;  // from <boost/beast/websocket.hpp>

typedef boost::asio::ip::tcp::socket tcp_socket;
typedef boost::beast::websocket::stream<boost::asio::ip::tcp::socket> websocket_stream;

// Base WS connection. Used for DSA connections over WS.
class WsConnection : public Connection {
  // write buffer will have 1/16 unusable part by default
  // which seems to improve the performance
  static const size_t DEFAULT_BUFFER_SIZE = 8192;
  static const size_t MAX_BUFFER_SIZE = DEFAULT_BUFFER_SIZE * 15;

  class WriteBuffer : public ConnectionWriteBuffer {
    WsConnection &connection;
    size_t size = 0;

   public:
    explicit WriteBuffer(WsConnection &connection) : connection(connection){};

    size_t max_next_size() const override;
    void add(const Message &msg, int32_t rid, int32_t ack_id) override;
    void write(WriteHandler &&callback) override;
  };

 protected:
  void read_loop_(shared_ptr_<WsConnection> &&connection, size_t from_prev,
                  const boost::system::error_code &error,
                  size_t bytes_transferred);

  std::vector<uint8_t> _read_buffer;
  std::vector<uint8_t> _write_buffer;
  websocket_stream _ws;

  std::atomic_bool _ws_open{true};

  void on_deadline_timer_(const boost::system::error_code &error,
                          shared_ptr_<Connection> sthis);

  void destroy_impl() override;

 public:
  WsConnection(websocket_stream &ws, LinkStrandRef &strand,
               const string_ &dsid_prefix, const string_ &path = "");

  void start_read(shared_ptr_<WsConnection> &&connection, size_t cur = 0,
                  size_t next = 0);

  std::unique_ptr<ConnectionWriteBuffer> get_write_buffer() override;

  websocket_stream &websocket();
};

}  // namespace dsa

#endif  // DSA_SDK_WS_CONNECTION_H_
