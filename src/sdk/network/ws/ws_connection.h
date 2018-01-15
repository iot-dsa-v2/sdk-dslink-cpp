#ifndef DSA_SDK_WS_CONNECTION_H_
#define DSA_SDK_WS_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/websocket.hpp>

#include "../base_socket_connection.h"
#include "util/enable_shared.h"

namespace dsa {

typedef boost::asio::ip::tcp::socket tcp_socket;
typedef boost::beast::websocket::stream<boost::asio::ip::tcp::socket>
    websocket_stream;

class WsConnection : public BaseSocketConnection {
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
  void continue_read_loop(shared_ptr_<Connection> &&sthis) final {
    start_read(std::move(sthis));
  }
  websocket_stream _socket;

  void destroy_impl() override;

 public:
  WsConnection(websocket_stream &ws, LinkStrandRef &strand,
               const string_ &dsid_prefix, const string_ &path = "");

  void start_read(shared_ptr_<Connection> &&connection) final;

  std::unique_ptr<ConnectionWriteBuffer> get_write_buffer() override;

  websocket_stream &socket();
};

}  // namespace dsa

#endif  // DSA_SDK_WS_CONNECTION_H_
