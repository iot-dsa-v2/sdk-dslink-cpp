#ifndef DSA_SDK_WEBSOCKET_H
#define DSA_SDK_WEBSOCKET_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

namespace websocket = boost::beast::websocket;
namespace http = boost::beast::http;

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

typedef websocket::stream<tcp::socket> websocket_stream;
typedef websocket::stream<ssl::stream<tcp::socket&>> websocket_ssl_stream;

namespace dsa {

class Websocket {
 private:
  bool _is_secure_stream;
  bool _is_websocket;
  tcp::socket _socket;
  std::unique_ptr<ssl::context> _ssl_context;
  std::unique_ptr<websocket_stream> _ws_stream;
  std::unique_ptr<websocket_ssl_stream> _wss_stream;

 public:
  Websocket(tcp::socket&& socket);
  Websocket(tcp::socket&& socket, ssl::context& ssl_context);
  bool is_secure_stream() { return _is_secure_stream; }
  bool is_websocket() { return _is_websocket; }
  void set_websocket() { _is_websocket = true; }
  tcp::socket& socket() { return _socket; }
  websocket_stream& stream() { return *_ws_stream; }
  websocket_ssl_stream& secure_stream() { return *_wss_stream; }

  template <typename... Args>
  void async_write(Args&&... args) {
    if (_is_websocket) return;

    if (_is_secure_stream) {
      http::async_write(_wss_stream->next_layer(), std::forward<Args>(args)...);
    } else {
      http::async_write(_ws_stream->next_layer(), std::forward<Args>(args)...);
    }
  }

  template <typename... Args>
  void shutdown(Args&&... args) {
    if (_is_websocket) return;

    if (_is_secure_stream) {
      _wss_stream->lowest_layer().shutdown(std::forward<Args>(args)...);
    } else {
      _ws_stream->lowest_layer().shutdown(std::forward<Args>(args)...);
    }
  }
};
}  // namespace dsa

#endif  // DSA_SDK_WEBSOCKET_H
