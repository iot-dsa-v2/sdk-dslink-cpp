#ifndef DSA_SDK_HTTP_CONNECTION_H
#define DSA_SDK_HTTP_CONNECTION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>
#include <memory>

namespace dsa {

class WebServer;
class WebsocketConnection;

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
 private:
  WebServer& _web_server;
  std::shared_ptr<WebsocketConnection> _connection;
  boost::asio::ip::tcp::socket _socket;
  boost::beast::flat_buffer _buffer;
  boost::beast::http::request<boost::beast::http::string_body> _req;
  boost::beast::http::response<boost::beast::http::dynamic_body> response_;
  boost::asio::basic_waitable_timer<std::chrono::steady_clock> deadline_;

 public:
  void process_request();
  void create_response();
  void write_response();
  void check_deadline();
  explicit HttpConnection(WebServer& web_server);
  void accept();
  boost::asio::ip::tcp::socket& socket() { return _socket; }
  boost::beast::http::request<boost::beast::http::string_body>& request() { return _req; }
  void destroy();
};
}  // namespace dsa

#endif  // DSA_SDK_HTTP_CONNECTION_H
