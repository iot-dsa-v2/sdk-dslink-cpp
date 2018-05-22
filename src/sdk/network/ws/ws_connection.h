#ifndef DSA_SDK_WS_CONNECTION_H
#define DSA_SDK_WS_CONNECTION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include "web_server/socket.h"
#include "../base_socket_connection.h"
#include "util/enable_shared.h"

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

namespace dsa {

typedef boost::asio::ip::tcp::socket tcp_socket;
typedef boost::beast::websocket::stream<boost::asio::ip::tcp::socket>
    websocket_stream;
typedef boost::beast::websocket::stream<ssl::stream<tcp_socket &>>
    websocket_ssl_stream;

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
  std::unique_ptr<Websocket> _websocket;

  void continue_read_loop(shared_ptr_<Connection> &&sthis) final {
    start_read(std::move(sthis));
  }

  void destroy_impl() override;

 public:
  WsConnection(const SharedLinkStrandRef &strand, const string_ &dsid_prefix,
               const string_ &path = "");

  Websocket &ws_stream() { return *_websocket; }

  void start_read(shared_ptr_<Connection> &&connection) final;

  std::unique_ptr<ConnectionWriteBuffer> get_write_buffer() override;
};

}  // namespace dsa

#endif  // DSA_SDK_WS_CONNECTION_H
