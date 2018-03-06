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

#include "websocket.h"

#include <memory>
#include <mutex>

#include "fields_alloc.h"

namespace http = boost::beast::http;

namespace dsa {

class Connection;
class WebServer;
class WebsocketConnection;
class HttpRequest;

// Web server side connection.
class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
 private:
  using alloc_t = fields_alloc<char>;
  using request_body_t = http::basic_dynamic_body<boost::beast::flat_static_buffer<1024 * 1024>>;
  WebServer& _web_server;
  bool _is_secured;
  shared_ptr_<Connection> _connection;

  boost::asio::ip::tcp::socket _socket;
  boost::beast::flat_static_buffer<8192> _buffer;
  alloc_t _alloc = alloc_t{8192};
  boost::optional<http::request_parser<request_body_t, alloc_t>> _parser;
  shared_ptr_<HttpRequest> _req;
  boost::asio::basic_waitable_timer<std::chrono::steady_clock> deadline_;
  std::unique_ptr<Websocket> _websocket;
  std::mutex _mutex;
  void reset_parser();

 public:
  void check_deadline();
  HttpConnection(WebServer& web_server, bool is_secured);
  void accept();
  boost::asio::ip::tcp::socket& socket() { return _socket; }
  auto request() { return _req; }
  void destroy();
};
}  // namespace dsa

#endif  // DSA_SDK_HTTP_CONNECTION_H
