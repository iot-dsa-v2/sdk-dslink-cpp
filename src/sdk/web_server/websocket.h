#ifndef DSA_SDK_WEBSOCKET_H
#define DSA_SDK_WEBSOCKET_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

namespace websocket =
    boost::beast::websocket;  // from <boost/beast/websocket.hpp>

using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;  // from <boost/asio/ssl.hpp>

typedef websocket::stream<ssl::stream<tcp::socket&>> websocket_ssl_stream;

namespace dsa {

class Websocket {
 private:
  bool _is_secure_stream;
  tcp::socket _socket;
  websocket_ssl_stream _wss_stream;

 public:
  Websocket(boost::asio::io_context& io_context, ssl::context& ssl_context);
  bool is_secure_stream() { return _is_secure_stream; }
  void set_secure_stream(bool value = true) { _is_secure_stream = value; };
  tcp::socket& socket() { return _socket; }
  websocket_ssl_stream& secure_stream() { return _wss_stream; }
};
}  // namespace dsa

#endif  // DSA_SDK_WEBSOCKET_H
