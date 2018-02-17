#ifndef DSA_SDK_WSS_CONNECTION_H_
#define DSA_SDK_WSS_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

#include "../base_socket_connection.h"
#include "util/enable_shared.h"

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

namespace dsa {

typedef boost::beast::websocket::stream<ssl::stream<tcp_socket &>>
    websocket_ssl_stream;

class WssConnection : public BaseSocketConnection {
  class WriteBuffer : public ConnectionWriteBuffer {
    WssConnection &connection;
    size_t size = 0;

   public:
    explicit WriteBuffer(WssConnection &connection) : connection(connection){};

    size_t max_next_size() const override;
    void add(const Message &msg, int32_t rid, int32_t ack_id) override;
    void write(WriteHandler &&callback) override;
  };

 protected:
  void continue_read_loop(shared_ptr_<Connection> &&sthis) final {
    start_read(std::move(sthis));
  }
  websocket_ssl_stream &_socket;

  void destroy_impl() override;

 public:
  WssConnection(websocket_ssl_stream &stream, LinkStrandRef &strand,
                const string_ &dsid_prefix, const string_ &path = "");

  void start_read(shared_ptr_<Connection> &&connection) final;

  std::unique_ptr<ConnectionWriteBuffer> get_write_buffer() override;

  websocket_ssl_stream &socket();
};

}  // namespace dsa

#endif  // DSA_SDK_WSS_CONNECTION_H_