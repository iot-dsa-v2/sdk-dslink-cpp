#ifndef DSA_SDK_HTTP_CONNECTION_H
#define DSA_SDK_HTTP_CONNECTION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>

#include "socket.h"
#include <memory>
#include <mutex>

namespace http = boost::beast::http;

namespace dsa {

class Connection;
class WebServer;

// Web server side connection.
class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
 private:
  WebServer& _web_server;
  bool _is_secured;
  shared_ptr_<Connection> _connection;

  boost::asio::ip::tcp::socket _socket;
  boost::beast::flat_buffer _buffer;
  boost::beast::http::request<boost::beast::http::string_body> _req;
  shared_ptr_ <Websocket> _websocket;
  std::mutex _mutex;

 public:
  HttpConnection(WebServer& web_server, bool is_secured);
  void accept();
  boost::asio::ip::tcp::socket& socket() { return _socket; }
  void destroy();
};
}  // namespace dsa

#endif  // DSA_SDK_HTTP_CONNECTION_H
