#ifndef DSA_SDK_HTTP_CONNECTION_H
#define DSA_SDK_HTTP_CONNECTION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>

#include <memory>

namespace dsa {

class WebServer;
class Connection;

// Web server side connection.
class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
 private:
  WebServer& _web_server;
  shared_ptr_<Connection> _connection;
  boost::asio::ip::tcp::socket _socket;
  boost::beast::flat_buffer _buffer;
  boost::beast::http::request<boost::beast::http::string_body> _req;

 public:
  HttpConnection(WebServer& web_server);
  void accept();
  boost::asio::ip::tcp::socket& socket() { return _socket; }
  void destroy();
};
}  // namespace dsa

#endif  // DSA_SDK_HTTP_CONNECTION_H
